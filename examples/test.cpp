#define WINVAL_IMPLEMENTATION
#include "Winval/Winval.h"
#include <unistd.h>
#include <iostream>

using namespace std;

int main(){
  char* p;
  int h = 600;
  int w = 800;
  Winval win(w, h, &p);
  
  for(int i = 0; i< h*w*4; i++){
    p[i] = 0xFF;
  }

  
  win.drawBuffer(p, w, h);
  while(1){
    sleep(1);
    win.flushEvents();
    cout<<win.isKeyPressed(WK_A)<<endl;
  }
  //cout<<win.waitForKey()<<endl;;
  return 0;

  
}
