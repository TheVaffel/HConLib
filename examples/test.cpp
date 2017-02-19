#define WINVAL_IMPLEMENTATION
#include "Winval/Winval.h"
#include <unistd.h>
#include <iostream>

using namespace std;

//Winval win;

int main(){
  char* dummy;
  int h = 600;
  int w = 800;
  
  char* p = new char[h*w*4];
  
  for(int i = 0; i< h*w*4; i++){
    p[i] = 0xFF;
  }

  
  Winval win(w, h, &dummy);
  win.drawBuffer(p, w, h);
  int down = 0;
  int x, y;
  while(1){
    for(int i = 0; i < h*w*4; i++){
      p[i] = ~p[i];
    }
    win.drawBuffer(p, w, h);
    win.waitForKey();
    if(win.isKeyPressed(WK_ESC))
       break;
    win.flushEvents();
    win.getPointerPosition(x, y);
    cout<<"Mouse is at "<<x<<", "<<y<<endl;
  }
  return 0;

  
}
