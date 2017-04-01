#define WINVAL_IMPLEMENTATION
#include <Winval/Winval.h>



#include <iostream>

using namespace std;

#define HGRAF_IMPLEMENTATION
#include <HGraf/HGraf.h>


int main(){
  int w = 400, h = 400;
  Winval win(w ,h);
  
  
  unsigned char* buffer = new unsigned char[4*w*h];
  for(hg::LineIterator li(0, 200, 200, 200); !li.isFinished(); li++){

 
    buffer[4*(li[1]*w + li[0])] = 0xFF;
  }
  
  win.drawBuffer(buffer, w, h);
  win.waitForKey();
  return 0;
}
