#include <stdio.h>
#include <Winval.h>

#include <iostream>

using namespace std;

int main(){
  int w = 400, h = 400;
  Winval win(w, h);

  unsigned char* image = new unsigned char[w*h*4];

  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      ((int*)image)[i*w + j] = 0xFF000000 + ((i*256/h)*0x1) + ((j*256/w)*0x100);
    }
  }
  win.drawBuffer(image, w, h);
  while(!win.isKeyPressed(WK_ESC)){
    win.flushEvents();
    int x, y;
    win.getPointerPosition(&x, &y);
    win.sleepMilliseconds(30);
  }

  return 0;
}
