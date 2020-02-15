#include <FlatAlg.hpp>

#include <iostream>

using namespace std;

int main() {
  falg::Matrix<2, 2> mat = falg::Matrix<2, 2>((falg::flatalg_t)1.0, (falg::flatalg_t)2.0,
					      (falg::flatalg_t)3.0, (falg::flatalg_t)4.0);
  cout << mat.str() << endl;

  int n = sizeof(falg::Matrix<2, 1>);
  cout << "Yeee" << endl;

  cout << "Size of Vec2 = " << n << endl;
}
