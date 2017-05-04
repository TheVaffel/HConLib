#define WINVAL_IMPLEMENTATION
#include "Winval/Winval.h"
#define HGRAF_IMPLEMENTATION
#include "HGraf/HGraf.h"
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
  win.setTitle("This is a test application");
  win.drawBuffer(p, w, h);
  int down = 0;
  int x, y;
	int bound[2][2] = {{0, 0}, {959, 539}};
	int point[2] = {958, 539};
	float vec[2] = {-0.972254, 0.233926};
	int res[2][2];

	hg::getBoundaryIntersections(point, vec, bound, res);
	cout<<"Got res "<<res[0][0]<<", "<<res[0][1]<<" and "<<res[1][0]<<", "<<res[1][1]<<endl;
 
 	hg::LineIterator li(959, 539, 0, 539);
	
	/*while(!li.isFinished()){
		cout<<li[0]<<", "<<li[1]<<endl;
		li++;
	}*/

	// cout<<li.currPoint[0]<<", "<<li.currPoint[1]<<" and "<<li.endPoint[0]<<", "<<li.endPoint[1]<<endl;
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
