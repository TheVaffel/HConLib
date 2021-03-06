#include <iostream>

#include <FlatAlg.hpp>

#include <cmath>

using namespace std;

int main(){
  falg::Mat3 rot = falg::Mat3(falg::FLATALG_MATRIX_ROTATION, 0.0f, 0.5f);

  falg::Vec3 rotvec = falg::Vec3(0.5, 0, 0);


  falg::Mat3 what(0, -rotvec[2], rotvec[1],
	rotvec[2], 0, -rotvec[0],
	-rotvec[1], rotvec[0], 0);

  falg::Mat3 rot2(falg::FLATALG_MATRIX_ROTATION, rotvec, 0.5f);

  cout<<"Rot2: "<<rot2.str()<<endl;
  
  cout<<rot.str()<<endl;
}
