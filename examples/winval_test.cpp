#include "Winval.h"
#include "HGraf.h"
#include <iostream>

using namespace std;

int main(){
  int h = 600;
  int w = 800;

  unsigned char* p = new unsigned char[h*w*4];

  for(int i = 0; i< h*w*4; i++){
    p[i] = 0xFF;
  }


  Winval win(w,h);
  win.setTitle("This is a test application");
  win.drawBuffer(p, w, h);
  int down = 0;
  int x = 0, y = 0;
  int bound[2][2] = {{0, 0}, {959, 539}};
  int point[2] = {958, 539};
  float vec[2] = {-0.972254f, 0.233926f};
  int res[2][2];

  hg::getBoundaryIntersections(point, vec, bound, res);
  cout<<"Got res "<<res[0][0]<<", "<<res[0][1]<<" and "<<res[1][0]<<", "<<res[1][1]<<endl;

  hg::LineIterator li(959, 539, 0, 539);

  /*while(!li.isFinished()){
    cout<<li[0]<<", "<<li[1]<<endl;
    li++;
    }*/

  win.setPointerVisible(false);
  // cout<<li.currPoint[0]<<", "<<li.currPoint[1]<<" and "<<li.endPoint[0]<<", "<<li.endPoint[1]<<endl;
  int lockX = w/2, lockY = h/2;
  win.lockPointer(true, lockX, lockY);
  while(win.isOpen()){
    /*for(int i = 0; i < h*w*4; i++){
      p[i] = ~p[i];
      }*/
    win.drawBuffer(p, w, h);
    //win.waitForKey();
    if(win.isKeyPressed(WK_ESC))
      break;
    win.flushEvents();
    int diffx, diffy;
    win.getPointerPosition(&diffx, &diffy);
    x += diffx - lockX;
    y += diffy - lockY;
    cout<<"X: "<<x<<", Y: "<<y<<endl;
  }

  return 0;


}
