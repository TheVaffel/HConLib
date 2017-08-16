#include <iostream>

#include <Winval.h>
#include <HCam.h>

using namespace std;

int w = 640, h = 480;

int main(){
  Winval win(w, h);
  HCam cam(w, h, "/dev/video0", HCAM_MODE_YUYV, false); //Last three arguments are optional
  int n;
  unsigned char buffer[w*h*4];
  
  while(win.isOpen()){
    cam.capture_image(buffer);
    win.drawBuffer(buffer, w, h);
    //win.waitForKey();
    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
  }

  return 0;
}
