#include <iostream>
#include <FlatAlg.hpp>

using namespace std;
using namespace falg;

int main(){
  float data[] = {
    1, 3, 1, 21, 48, 87,
    48, -14, 18, 84, 8, 8,
    18, 49, 78, 2, 99, 67,
    89, -1414, 8, 80, 30, 38,
    93, 13, 0, 1, 3, 67,
    28, 3414, 46, 27, 96, 8
  }; 
  /* float data[] = {
    1, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1
    }; */

  float data2[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
  
  // Matrix mat(6, 6, data);

  Matrix<6, 6> mat(FLATALG_MATRIX_FROM_DATA, data);

  Matrix<4, 4> anotherMat(FLATALG_MATRIX_FROM_DATA, data2);
  Matrix<4, 4> anotherInv = anotherMat.inv();
  
  Matrix<6, 6> mm = mat.inv();

  cout << "Other matrix: \n" << anotherMat.str() << endl;
  cout << "Other determinant: " << anotherMat.det() << endl;
  cout << "Other matrix' inverse: \n" << anotherInv.str() << endl;
  cout << "Other product: \n" << (anotherMat * anotherInv).str() << endl;
  
  cout << "This is the original matrix: \n" << mat.str() << endl;
  cout << "Its determinant: \n" << mat.det() << endl;
  
  cout<<"This is our inverse:\n"<<mm.str()<<endl;

  cout<<"The product is "<<(mat*mm).str()<<endl;

  Vec3 p(3.34, -13, 1.1310);
  
  Quaternion q(1, Vec3(3.03, 1.23, 3.94).normalized());
  Mat3 rotation1 = q.toMatrix();

  cout << "Quaternion: " << q.str() << endl;
  cout << "Matrix: " << rotation1.str() << endl;
  cout << "Matrix to quaternion " << Quaternion(rotation1).str() << endl;

  Quaternion anotherQuaternion(0.9999 * F_PI, Vec3(100, 2, 1).normalized());
  Mat3 anotherMatrix = anotherQuaternion.toMatrix();
  cout << "New quaternion: " << anotherQuaternion.str() << endl;
  cout << "Another matrix: " << anotherMatrix.str() << endl;
  cout << "Another matrix to quaternion: " << Quaternion(anotherMatrix).str() << endl;
  Vec3 rotated1 = q.rotate(p);
  Vec3 rotated2 = rotation1 * p;

  cout << "Matrix rotated: " << rotated2.str() << endl;
  cout << "Quaternion rotated: " << rotated1.str() << endl;

  Vec3 translation(14.2, 14.4, 4.21);
  DualQuaternion dual(q, translation);
  Mat4 matmat = dual.toMatrix();
  

  Vec3 checkTransform = q.rotate(p) + translation;
  Vec3 dualTransformed = dual.transform(p);
  Vec3 matrixTransformed = matmat * p;

  Vec3 justRotated = q.rotate(p);
  Vec3 justRotatedMat = q.toMatrix() * p;


  cout << "Just rotated: " << justRotated.str() << endl;
  cout << "Just rotated mat: " << justRotatedMat.str() << endl;

  cout << "Sanity check transformed: " << checkTransform.str() << endl;
  cout << "Dual transformed: " << dualTransformed.str() << endl;
  cout << "Matrix transformed: " << matrixTransformed.str() << endl;
  cout << "Original dual quaternion: " << dual.str() << endl;
  cout << "Converted to matrix: " << matmat.str() << endl;
  cout << "Inverted matrix: " << DualQuaternion(matmat).str() << endl;

  Vec3 v(1, 3, 4);
  Vec3 v2(4, 5, 3);

  cout << "Vector 1: " << v.str() << ", Vector2: " << v2.str() << endl;

  v += v2;
  v2 -= v;
  
  cout << "After v1 += v2, v2 -= v1: v1 = " << v.str() << ", v2 = " << v2.str() << endl;

  Mat3 testm(1, 2, 3,
		4, 5, 6,
		7, 8, 9);
  cout << "Testmat: " << testm.str() << endl;
  cout << " - Testmat: " << (- testm).str() << endl;

  Mat2 testm2(1, 4, 5, 7);

  cout << "Testmat2: " << testm2.str() << endl;
  cout << " - Testmat2: " << ( - testm2).str() << endl;
  
  return 0;
  
}
