#include <Flaudio.h>
#include <unistd.h>

Flaudio::Flaudio(unsigned int rate){
  samplerate = rate;

  channels = std::vector<std::queue<Segment*> >(1);
  
  int dir;

  /* Open PCM device for playback. */
  int rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /*Setting sampling rate */
  
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &rate, &dir);

  /* Set period size to 128 frames. */
  frames_per_period = 128;
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames_per_period,
					 &dir);

  rc = snd_pcm_hw_params(handle, params);
  if(rc < 0){
    fprintf(stderr,
	    "unable to set hw parameters in Flaudio constructor: %s\n",
	    snd_strerror(rc));
    exit(1);
  }

  total_buffer_size = snd_pcm_avail(handle);
}


void Flaudio::writeBuffer(int16_t* buffer, int num){
  int16_t* newbuff = new int16_t[num*2];
  int16_t* a = newbuff;
  for(int i = 0; i < num; i++){
    *(a++) = buffer[i];
    *(a++) = buffer[i];
  }
  a = newbuff;
  while(num != 0){
    snd_pcm_writei(handle, a, frames_per_period);
    num = num < frames_per_period? 0: num - frames_per_period;
    a += frames_per_period*2;
  }
  delete[] newbuff;
  
}

void Flaudio::playStep(){

  
  long int delay = 0;
  snd_pcm_delay(handle, &delay);
  
  while(delay > 3*frames_per_period/2){
    usleep(1000); //Sleep 1 ms at a time
    int err = snd_pcm_delay(handle, &delay);
    if(err < 0){
      int u = snd_pcm_recover(handle, err, 1);
      if(u < 0){
	printf("Recovery failed\n");
	exit(0);
      }
    }
    
    if(snd_pcm_state(handle) != SND_PCM_STATE_RUNNING){
      snd_pcm_prepare(handle);
    }
  }
  //int numberToWrite = frames_per_period - (total_buffer_size - av);
  //printf("NumberToWrite = %d, computed from %ld, %d and %d\n", numberToWrite, frames_per_period, total_buffer_size, av);

  int numberToWrite = frames_per_period;
  
  if(numberToWrite == 0){
    return;
  }
  //int16_t newbuff[100000];
  int16_t* newbuff = new int16_t[numberToWrite*2];
  for(int i = 0; i < 2*numberToWrite ;i++){
    newbuff[i] = 0;
  }

  for(int j = 0; j < channels.size(); j++){
    if(!channels[j].empty()){
      for(int i =0 ;i < numberToWrite; i++){
	Segment* s = channels[j].front();
	if(!s->isStereo){
	  newbuff[2*i] += s->samples[s->currSample++];
	  newbuff[2*i + 1] += newbuff[2*i];
	}else{
	  newbuff[2*i] += s->samples[s->currSample++];
	  newbuff[2*i + 1] += s->samples[s->currSample++];
	}
	if(s->currSample == s->numSamples){
	  if(s->repeat){
	    s->currSample = 0;
	  }else{
	    delete channels[j].front();
	    channels[j].pop();
	    if(channels[j].empty()){
	      break;
	    }
	  }
	}
      }
    }
  }

  /*for(int i = 0; i < frames_per_period; i++){
    printf("Writing %d\n", newbuff[2*i]);
    }*/

  //snd_pcm_delay(handle, &delay);
  //printf("Delay is %ld frames\n", delay);
  
  int written = snd_pcm_writei(handle, newbuff, numberToWrite);
  if(written < 0){
    
    int u = snd_pcm_recover(handle, written, 1);
    if(u < 0){
      printf("Recover failed\n");
      exit(0);
    }
  }
  
  delete[] newbuff;
}

int Flaudio::getSampleRate(){
  return samplerate;
}

int Flaudio::getSamplesPerStep(){
  return frames_per_period;
}

int Flaudio::getNumChannels(){
  return channels.size();
}

bool Flaudio::isChannelEmpty(int u){
  return u<getNumChannels()?channels[u].empty():0;
}

void Flaudio::endSegmentInChannel(int u){
  if(channels[u].size()){
    channels[u].front()->repeat = 0;
  }
  
}

void Flaudio::enqueueToChannel(int16_t* buffer, int length, int channel, bool repeating){
  Segment* s = new Segment(length, repeating);
  memcpy(s->samples, buffer, length*2);
  
  channels[channel].push(s);
}

int Flaudio::enqueueToSuitableChannel(int16_t* buffer, int length, bool repeating){
  
  for(int i = 0; i < channels.size(); i++){
    if(channels[i].size()==0){
      enqueueToChannel(buffer, length, i, repeating);
      return i;
    }
  }
  channels.push_back(std::queue<Segment*>());
  enqueueToChannel(buffer, length, channels.size() - 1, repeating);
  return channels.size()-1;
}

Flaudio::~Flaudio(){
  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  for(int i = 0; i  < channels.size();i++){
    while(!channels[i].empty()){
      if(channels[i].front()){
	delete channels[i].front();
      }
      channels[i].pop();
    }
  }
}
