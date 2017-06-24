#include <iostream>

#define WINVAL_IMPLEMENTATION
#include <Winval/Winval.h>
#include <webcam/webcam.h>

using namespace std;

int w = 640, h = 480;

int main(){
  Winval win(w, h);
  webcam_init(w, h, 0, WEBCAM_MODE_YUYV, true);
  int n;
  unsigned char buffer[w*h*4];
  
  while(true){
    webcam_capture_image(buffer);
    win.drawBuffer(buffer, w, h);
    //win.waitForKey();
    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
  }

  return 0;
}
