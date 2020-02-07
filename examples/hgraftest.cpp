#include "HGraf.hpp"
#include <iostream>
#include "Winval.hpp"
#include <cmath>

using namespace std;
using namespace falg;

int main(){
  int w = 1000, h =  1000;
  Winval win(w, h);

  hg::Canvas vas(w, h);

  hg::CamParam par(w, h, F_PI/2, 0.1f);
  for(int i = 0; i < w*h; i++){
    vas[i] = 0;
  }
  win.drawBuffer(vas.getData(), w, h);
  //win.waitForKey();
  /*for(int i = 0; i < 1000; i++){
    usleep(50000);
    cout<<"Iterated "<<i<<endl;
    hg::drawLine3D(vas, par, Point3(1.5*sin(i*0.03), 1.5*cos(i*0.03), -2), Point3(0.5*sin(i*0.1), 0.5*cos(i*0.1), -1), 0xFF0000 +  (i%256)*(1 + (1<<8)));
    win.drawBuffer(vas.getData(), w, h);
  }*/
  Mat4 m(FLATALG_MATRIX_TRANSLATION, Vec3(0,0,-5));
  Mat4 r(FLATALG_MATRIX_IDENTITY);
  Mat4 rr(FLATALG_MATRIX_ROTATION_Y, 0.05f);
  int i = 0;
  hg::LineCube lc(1, 1, 1);
  while(win.isOpen() && !win.isKeyPressed(WK_ESC)){
    r = r*rr;
    m = Mat4(FLATALG_MATRIX_TRANSLATION, Vec3(0,0,-3 + 1.5f*cos(i*0.07f)));
    cout << "Translation matrix: " << m.str() << endl;
    cout << "Rotation matrix: " << r.str() << endl;
    
    m = m*r;

    cout << "Matrix sent to draw function: " << m.str() << endl;
    hg::clearCanvas(vas);
    hg::drawLineModel(vas, par, lc, m, 0xFF0000);
    win.drawBuffer(vas.getData(), w, h);
    win.sleepMilliseconds(20);
    ++i;
  }
  //win.waitForKey();
  return 0;
}
