#include <iostream>

#define FLATALG_IMPLEMENTATION
#include <FlatAlg/FlatAlg.h>

#include <cmath>

using namespace std;

int main(){
  Matrix3 rot = Matrix3(FLATALG_MATRIX_ROTATION, 0.0, 0.5);

  Vector3 rotvec = Vector3(0.5, 0, 0);


  Matrix3 what(0, -rotvec[2], rotvec[1],
	rotvec[2], 0, -rotvec[0],
	-rotvec[1], rotvec[0], 0);

  Matrix3 rot2(FLATALG_MATRIX_ROTATION, rotvec, 0.5);

  cout<<"Rot2: "<<rot2.str()<<endl;
  
  cout<<rot.str()<<endl;
}
