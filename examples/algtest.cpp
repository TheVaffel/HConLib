#include <iostream>
#include <FlatAlg.hpp>

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

  Vector3 p(3.34, -13, 1.1310);
  
  Quaternion q(1, Vector3(3.03, 1.23, 3.94).normalized());
  Matrix3 rotation1 = q.toMatrix();

  cout << "Quaternion: " << q.str() << endl;
  cout << "Matrix: " << rotation1.str() << endl;
  cout << "Matrix to quaternion " << rotation1.toQuaternion().str() << endl;

  Quaternion anotherQuaternion(0.9999 * F_PI, Vector3(100, 2, 1).normalized());
  Matrix3 anotherMatrix = anotherQuaternion.toMatrix();
  cout << "New quaternion: " << anotherQuaternion.str() << endl;
  cout << "Another matrix: " << anotherMatrix.str() << endl;
  cout << "Another matrix to quaternion: " << anotherMatrix.toQuaternion().str() << endl;
  Vector3 rotated1 = q.rotate(p);
  Vector3 rotated2 = rotation1 * p;

  cout << "Matrix rotated: " << rotated2.str() << endl;
  cout << "Quaternion rotated: " << rotated1.str() << endl;

  Vector3 translation(14.2, 14.4, 4.21);
  DualQuaternion dual(q, translation);
  Matrix4 matmat = dual.toMatrix();
  

  Vector3 checkTransform = q.rotate(p) + translation;
  Vector3 dualTransformed = dual.transform(p);
  Vector3 matrixTransformed = matmat * p;

  Vector3 justRotated = q.rotate(p);
  Vector3 justRotatedMat = q.toMatrix() * p;


  cout << "Just rotated: " << justRotated.str() << endl;
  cout << "Just rotated mat: " << justRotatedMat.str() << endl;

  cout << "Sanity check transformed: " << checkTransform.str() << endl;
  cout << "Dual transformed: " << dualTransformed.str() << endl;
  cout << "Matrix transformed: " << matrixTransformed.str() << endl;
  cout << "Original dual quaternion: " << dual.str() << endl;
  cout << "Inverted matrix: " << matmat.toDualQuaternion().str() << endl;

  Vector3 v(1, 3, 4);
  Vector3 v2(4, 5, 3);

  cout << "Vector 1: " << v.str() << ", Vector2: " << v2.str() << endl;

  v += v2;
  v2 -= v;
  
  cout << "After v1 += v2, v2 -= v1: v1 = " << v.str() << ", v2 = " << v2.str() << endl;

  Matrix3 testm(1, 2, 3,
		4, 5, 6,
		7, 8, 9);
  cout << "Testmat: " << testm.str() << endl;
  cout << " - Testmat: " << (- testm).str() << endl;
  
  return 0;
  
}
