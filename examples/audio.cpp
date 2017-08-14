#include <iostream>
#include <Flaudio.h>

#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;

int main(){
  Flaudio fl;
  int sampleRate = fl.getSampleRate();
  int numSamples = 1*sampleRate;
  
  int samplesPerStep = fl.getSamplesPerStep();
  int16_t* samples = new int16_t[samplesPerStep];

  float freq = 440;
  float maxVol = 1<<13;


  for(int j = 0; j < numSamples/samplesPerStep; j++){
    
    for(int i = 0; i < samplesPerStep; i++){
      samples[i] = (int16_t)(maxVol*sin(freq*(j*samplesPerStep + i)/sampleRate*2*M_PI));
    }
    
    fl.enqueueToChannel(samples, samplesPerStep, 0);
    fl.playStep();
  }

  /*for(int i = 0; i < numSamples/samplesPerStep; i++){
    fl.playStep();
    }*/

  return 0;
  
 
  
}
