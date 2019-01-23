#include <iostream>

#include <HCam.hpp>
#include <Winval.hpp>

using namespace std;

const int w = 640, h = 480;

int main(){
  Winval win(w, h);
  HCam cam(w, h, "/dev/video1", HCAM_MODE_YUYV, false); //Last three arguments are optional
  //Also, the "mode"-argument and width/height does nothing in Windows.
  //You have to run the program and see the printed width and height before you go on
  //I don't like Windows either
  unsigned char * buffer = new unsigned char[w*h*4];
  while(win.isOpen()){
    cam.capture_image(buffer);
    win.drawBuffer(buffer, w, h);
    //win.waitForKey();
    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
  }

  delete[] buffer;

  return 0;
}
