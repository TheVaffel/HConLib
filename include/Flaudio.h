
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
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
  
  snd_pcm_uframes_t frames_per_period;
  unsigned int samplerate;
  
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;

  int total_buffer_size;
  
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

