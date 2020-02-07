#define FLATALG_IMPLEMENTATION
#include <FlatAlg.hpp>

#include <iostream>

using namespace std;

int main(){
  falg::Vec3 v(100, 4, 10);


	cout<<v.normalized().str()<<endl;

	falg::Mat3 mat(1, 0, 3,
		3, 2, 5,
		4, 2, 4);
	falg::Mat3 mat_inv = mat.inv();
       
	cout << "Matrix: \n" << mat.str() << endl;
	cout << "Its inverse: \n" << mat_inv.str() << endl;
	cout << "Their product: \n" << (mat_inv * mat).str() << endl;
	return 0;
	
}
