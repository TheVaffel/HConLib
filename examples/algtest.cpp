#include <iostream>
#define FLATALG_IMPLEMENTATION
#include "FlatAlg/FlatAlg.h"

using namespace std;
using namespace flatalg;


int main(){
  float data[] = {
    1, 3, 41, 21, 48, 87,
    48, 48, 18, 84, 8, 8,
    18, 49, 78, 2, 99, 67,
    89, 63, 8, 80, 30, 38,
    93, 13, 0, 1, 3, 67,
    28, 39, 46, 27, 96, 8
  };
  
  Matrix mat(6, 6, data);

  Matrix mm = mat.inv();
  
  cout<<"This is our inverse:\n"<<mm.str()<<endl;

  cout<<"The product is "<<(mat*mm).str()<<endl;

  Matrix3 rot(FLATALG_MATRIX_ROTATION,
		      Vector3(1, 0, -1),
		      1.0f);
  cout<<"The determinant of the rotation matrix "<<rot.str()<<" is "<<rot.det()<<endl; 
  return 0;
  
}
