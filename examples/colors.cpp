#include <stdio.h>
#include <Winval.h>

#include <iostream>

#ifdef WIN32
void sleepMilliseconds(int m){
  Sleep(m);
}
#else //WIN32
#include <unistd.h>
void sleepMilliseconds(int m){
  usleep(m*1000);
}
#endif //WIN32

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
    sleepMilliseconds(30);
  }

  return 0;
}
