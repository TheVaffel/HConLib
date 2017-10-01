#include <Winval.h>
#include <Flaudio.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

using namespace std;

int main(){
  const int w = 400, h = 400;

  Winval win(w, h);

  const int charcodes[] = {WK_A, WK_O, WK_E, WK_U, WK_I, WK_D, WK_H, WK_T, WK_N, WK_S};
  const int steps[] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16};

  const int numKeys = 10;

  bool isPressed[numKeys];
  int channels[numKeys];
  for(int i =0; i < numKeys; i++){
    isPressed[i] = 0;
    channels[i] = -1;
  }




  Flaudio fl;

  int samplerate = fl.getSampleRate();
  int16_t *tempsamples= new int16_t[samplerate];
  int16_t maxvol = 1<<11;


  /*int16_t* test  = new int16_t[samplerate];
  float fruq = 440;
  for(int i= 0;i < samplerate; i++){
    test[i] = (int16_t)(maxvol*sin(fruq*i/samplerate*2*M_PI));
  }

  fl.enqueueToSuitableChannel(test, samplerate, true);*/
  /*while(true){
    fl.playStep();
    }*/

  win.enableAutoRepeat(true);
  int it = 0;
  while(win.isOpen()){
    it++;
    win.flushEvents();
    //cout<<"Flushed events "<<it<<endl;
    for(int i = 0; i < numKeys; i++){
      if(!isPressed[i] && win.isKeyPressed(charcodes[i])){
	float freq = 440*powf(2.0f, (steps[i] - 9.0f)/12.0f);
	int buffsize = (int)(samplerate/freq);
	for(int p =0; p < buffsize; p++){
	  tempsamples[p] = (int16_t)(maxvol*sin(freq*p/samplerate*2*M_PI));
	}
	channels[i] = fl.enqueueToSuitableChannel(tempsamples, buffsize, true);
	isPressed[i] = true;
	//cout<<"Started in channel "<<channels[i]<<endl;
      }else if(isPressed[i] && !win.isKeyPressed(charcodes[i])){
	fl.endSegmentInChannel(channels[i]);

	//cout<<"Ended in channel "<<channels[i]<<endl;
	channels[i] = -1;
	isPressed[i] = false;
      }
    }
    //cout<<"Playing step"<<endl;
    fl.playStep();
    //cout<<"Played step"<<endl;

    //printf("Checking esc %d\n", it);
    if(win.isKeyPressed(WK_ESC)){
      return 0;
    }
  }

  return 0;
}
