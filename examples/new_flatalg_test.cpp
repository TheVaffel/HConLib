#include <FlatAlg.hpp>

#include <iostream>

using namespace std;

int main() {
  Matrix<2, 2> mat = Matrix<2, 2>((flatalg_t)1.0, (flatalg_t)2.0,
				  (flatalg_t)3.0, (flatalg_t)4.0);
  cout << mat.str() << endl;

  int n = sizeof(Matrix<2, 1>);
  cout << "Yeee" << endl;

  cout << "Size of vector2 = " << n << endl;
}
