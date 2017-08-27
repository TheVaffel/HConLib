
#ifndef INCLUDED_FLAUDIO
#define INCLUDED_FLAUDIO

#ifdef WIN32

#include <windows.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

#else //WIN32
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#endif //WIN32

#include <queue>
#include <vector>

class Flaudio{
  struct Segment{
    int16_t* samples;
    int numSamples;
    bool isStereo;
    int currSample;
    bool repeat;

    Segment(int size, bool repeatQ, bool stereo = false){
      numSamples = size;
      samples = new int16_t[size];
      isStereo = stereo;
      currSample = 0;
      repeat = repeatQ;
    }

    ~Segment(){
      if(samples){
	delete[] samples;
      }
    }
  };

  std::vector<std::queue<Segment*> > channels;

  unsigned int samplerate;

#ifdef WIN32

  WAVEFORMATEX* format;
  IMMDevice* device;
  IAudioClient * audioClient;
  IAudioRenderClient * renderClient;

#else //WIN32
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
#endif //WIN32

  unsigned int total_buffer_size;
  unsigned int frames_per_period;

public:

  Flaudio(unsigned int samplerate = 44100);
  ~Flaudio();

  void writeBuffer(int16_t*buffer, int num);
  void playStep();

  int getSampleRate();
  int getSamplesPerStep();
  int getNumChannels();
  bool isChannelEmpty(int u);

  void enqueueToChannel(int16_t* buffer, int length, int channel, bool repeat = false);
  int enqueueToSuitableChannel(int16_t* buffer, int length, bool repeat = false);
  void endSegmentInChannel(int channel);
};

#endif //INCLUDED_FLAUDIO
