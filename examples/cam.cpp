#include <iostream>

#include <Winval.h>
#include <webcam.h>

using namespace std;

int w = 640, h = 480;

int main(){
  Winval win(w, h);
  webcam_init(w, h, 0);
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
