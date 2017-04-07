//FlatAlg.h - Haakon Flatval

#ifndef INCLUDED_FLATALG
#define INCLUDED_FLATALG
#include <string>

struct Point2{
private:
  float len;
public:
  union{
    struct{float x, y;};
    float p[2];
  };


  float& operator[](int a);

  float get(int a) const;

  Point2();
  
  Point2(float nx, float ny);
  void normalize();
  float length();

  std::string str() const;
};

typedef Point2 Vector2;

struct Matrix2{
  float mat[9];

  float* operator[](int a);
  Matrix2 invert();
  float det();
  float get(int a, int b) const;

  Matrix2(int i);
  Matrix2(int i, float theta);

  Matrix2();
  Matrix2(float a1, float a2,
	  float a3, float a4);

  std::string str() const;
};

float cross(const Vector2& v1, const Vector2& v2);

float operator*(const Vector2& v1, const Vector2& v2);

Vector2 operator*(const Vector2& v1, float f);

Vector2 operator*(float f, const Vector2& v1);

Vector2 operator+(const Vector2& v1, const Vector2& v2);

Vector2 operator-(const Vector2& v1, const Vector2& v2);

Vector2 operator/(const Vector2& v, float f);

Vector2 operator*(const Matrix2& m, const Vector2& v);

Vector2 operator*(const Vector2& v, const Matrix2& m);

Matrix2 operator~(const Matrix2& m);

Matrix2 operator*(const Matrix2& m1, const Matrix2& m2);

Matrix2 operator*(const Matrix2& m1, float f);

Matrix2 operator*(float f, const Matrix2& m1);


struct Point3{
private:
  float len;
  
public:
  union{
    struct{float x, y, z;};
    float p[3];
  };
  
  float& operator[](int a);

  float get(int a) const;

  Point3();

  Point3(float nx, float ny, float nz);

  void normalize();
  float length();

  std::string str() const;
};

typedef Point3 Vector3;

struct Matrix3{
  float mat[9];

  float* operator[](int a);

  float det() const;

  float get(int a, int b) const;

  Matrix3(int i);

  Matrix3(int i, float theta);

  Matrix3(int i, float arg1, float arg2);

  
  Matrix3(int i, const Vector3& v, float arg1);

  Matrix3();
  Matrix3(float a1, float a2, float a3,
	  float a4, float a5, float a6,
	  float a7, float a8, float a9);
  

  std::string str() const;
};

Vector3 cross(const Vector3& v1, const Vector3& v2);

float operator*(const Vector3& v1, const Vector3& v2);

Vector3 operator*(const Vector3& v, float f);

Vector3 operator*(float f, const Vector3& v);

Vector3 operator+(const Vector3& v1, const Vector3& v2);

Vector3 operator-(const Vector3& v1, const Vector3& v2);

Vector3 operator/(const Vector3& v, float f);

Vector3 operator*(const Matrix3& m, const Vector3& v);

Vector3 operator*(const Vector3& v, const Matrix3& m);

Matrix3 operator+(const Matrix3& m1, const Matrix3& m2);

Matrix3 operator*(const Matrix3& m, float f);

Matrix3 operator*(float f, const Matrix3& m);

Matrix3 operator/(const Matrix3& m, float f);

Matrix3 operator~(const Matrix3& m);

Matrix3 operator*(const Matrix3& m1, const Matrix3& m2);

struct Matrix4{
  float mat[16];

  float* operator[](int a);

  float det();

  float get(int a, int b) const;
  float get(int a) const;
  
  Matrix4(int i);

  Matrix4(int i, float theta);

  Matrix4(int i, float arg1, float arg2);

  Matrix4(int i, const Point3& p);

  Matrix4();
  Matrix4(float a1, float a2, float a3, float a4,
	  float a5, float a6, float a7, float a8,
	  float a9, float a10, float a11, float a12,
	  float a13, float a14, float a15, float a16);

  std::string str() const;
};

Vector3 operator*(const Matrix4& m, const Vector3& v);

Vector3 operator*(const Vector3& v, const Matrix4& m);

Matrix4 operator*(const Matrix4& m, float f);

Matrix4 operator*(float f, const Matrix4& m);

Matrix4 operator~(const Matrix4& m);

Matrix4 operator*(const Matrix4& m1, const Matrix4& m2);

struct GeneralVector{
  float* vec;
  int l;

  float& operator[](int a);
  int getLength() const;
  int getL() const;
  float get(int a) const;
  GeneralVector(int l);
  GeneralVector(int l, const float* p);
  GeneralVector(const GeneralVector& gv);
  ~GeneralVector();

  std::string str() const;
};

float operator*(const GeneralVector& g1, const GeneralVector& g2);

GeneralVector operator+(const GeneralVector& g1, const GeneralVector& g2);

GeneralVector operator-(const GeneralVector& g1, const GeneralVector& g2);

GeneralVector operator*(const GeneralVector& g1, float f);

GeneralVector operator*(float f, const GeneralVector& g1);

GeneralVector operator/(const GeneralVector& g1, float f);



struct GeneralMatrix{
  float* mat;
  int w, h;

  float* operator[](int a);

  float get(int a, int b) const;
  float get(int a) const;
  int getWidth() const;
  int getHeight() const;
  int getW() const;
  int getH() const;
  float det() const;
  float _det() const;
  GeneralMatrix inv() const;
  GeneralMatrix(int w, int h);
  GeneralMatrix(int w, int h, const float* p);
  GeneralMatrix(const GeneralMatrix& gm);
  ~GeneralMatrix();
  
  GeneralMatrix _minor(int a, int b) const;

  std::string str() const;
};

GeneralVector operator*(const GeneralMatrix& gm, const GeneralVector& gv);

GeneralVector operator*(const GeneralVector& gv, const GeneralMatrix& gm);

GeneralMatrix operator~(const GeneralMatrix& gm);

GeneralMatrix operator*(const GeneralMatrix& gm, float f);

GeneralMatrix operator/(const GeneralMatrix& gm, float f);

GeneralMatrix operator*(float f, const GeneralMatrix& gm);

GeneralMatrix operator*(const GeneralMatrix& g1, const GeneralMatrix& g2);

#endif // INCLUDED_FLATALG

#ifdef FLATALG_IMPLEMENTATION

#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream> // For human-readable representations

#define FLATALG_MATRIX_IDENTITY 0
//Rotations around: 1. Z-axis  2. X-axis 3. Y-axis
#define FLATALG_MATRIX_ROTATION 1
#define FLATALG_MATRIX_TRANSLATION 2
#define FLATALG_MATRIX_ROTATION_X 3
#define FLATALG_MATRIX_ROTATION_Y 4
#define FLATALG_MATRIX_ROTATION_Z 5

float& Point2::operator[](int a){
  len = -1;
  return p[a];
}

float Point2::get(int a) const{
  return p[a];
}

Point2::Point2(){
  len = 0;
  p[0] = 0; p[1] = 0;
}
  
Point2::Point2(float nx, float ny){
  len = -1;
  p[0] = nx; p[1] = ny;
}

void Point2::normalize(){
  if(len == 1)
    return;
  float inv = 1/length();
  len = 1;
  x *= inv; y *= inv;
}

float Point2::length(){
  return len>=0?len:(len = sqrt(x*x + y*y));
}

std::string Point2::str() const {
  std::ostringstream oss;
  oss<<"["<<p[0]<<", "<<p[1]<<"]";
  return oss.str();
}


float* Matrix2::operator[](int a){
  return mat + 2*a;
}

Matrix2 Matrix2::invert(){
  return (1/this->det())*Matrix2(mat[3], -mat[1],
				 -mat[2], mat[0]);
}

float Matrix2::det(){
  return mat[0]*mat[3] - mat[1]*mat[2];
}

float Matrix2::get(int a, int b) const{
  return mat[2*b + a];
}


Matrix2::Matrix2(int i){
  switch(i){
  case FLATALG_MATRIX_IDENTITY:
  default:
    mat[0] = 1; mat[1] = 0;
    mat[2] = 0; mat[3] = 1;
    break;
  }
}

Matrix2::Matrix2(int i, float theta){
  switch(i){
  case FLATALG_MATRIX_ROTATION:
  default:
    float st = sin(theta), ct = cos(theta);
    mat[0] = ct; mat[1] = -st;
    mat[2] = st; mat[3] = ct;
  }
}

Matrix2::Matrix2(){
  mat[0] = 1; mat[1] = 0;
  mat[2] = 0; mat[3] = 1;
}

Matrix2::Matrix2(float a1, float a2,
		 float a3, float a4){
  mat[0] = a1; mat[1] = a2;
  mat[2] = a3; mat[3] = a4;
}

std::string Matrix2::str() const {
  std::ostringstream oss;
  oss<<"["<<mat[0]<<", "<<mat[1]<<"]"<<std::endl
     <<"["<<mat[2]<<", "<<mat[3]<<"]";
  return oss.str();
}


float cross(const Vector2& v1, const Vector2& v2){
  return v1.get(0)*v2.get(1) - v1.get(1)*v2.get(0);
}

float operator*(const Vector2& v1, const Vector2& v2){
  return v1.get(0)*v2.get(0) + v1.get(1)*v2.get(1);
}

Vector2 operator*(const Vector2& v1, float f){
  return Vector2(v1.get(0)*f, v1.get(1)*f);
}

Vector2 operator*(float f, const Vector2& v1){
  return Vector2(v1.get(0)*f, v1.get(1)*f);
}

Vector2 operator+(const Vector2& v1, const Vector2& v2){
  return Vector2(v1.get(0) + v2.get(0), v1.get(1) + v2.get(1));
}

Vector2 operator-(const Vector2& v1, const Vector2& v2){
  return Vector2(v1.get(0) - v2.get(0), v1.get(1) - v2.get(1));
}

Vector2 operator/(Vector2& v, float f){
  float inv = 1/f;
  return Vector2(v.get(0)*inv, v.get(1)*inv);
}

Vector2 operator*(const Matrix2& m, const Vector2& v){
  return Vector2(m.get(0, 0)*v.get(0) + m.get(1, 0)*v.get(1), m.get(0, 1)*v.get(0) + m.get(1, 1)*v.get(1));
}

Vector2 operator*(const Vector2& v, const Matrix2& m){
  return Vector2(v.get(0)*m.get(0, 0) + v.get(1)*m.get(0, 1), v.get(0)*m.get(1, 0) + v.get(1)*m.get(1, 1));
}

Matrix2 operator~(const Matrix2& m){
  return Matrix2(m.get(0, 0), m.get(0, 1),
		 m.get(1, 0), m.get(1, 1));
}

Matrix2 operator*(const Matrix2& m1, const Matrix2& m2){
  return Matrix2(m1.get(0, 0)*m2.get(0, 0) + m1.get(1, 0)*m2.get(0, 1),
		 m1.get(0, 0)*m2.get(1, 0) + m1.get(1, 0)*m2.get(1, 1),

		 m1.get(0, 1)*m2.get(0, 0) + m1.get(1, 1)*m2.get(0, 1),
		 m1.get(0, 1)*m2.get(1, 0) + m1.get(1, 1)*m2.get(1, 1));
}

Matrix2 operator*(const Matrix2& m1, float f){
  return Matrix2(m1.get(0, 0)*f, m1.get(1, 0)*f,
		 m1.get(0, 1)*f, m1.get(1, 1)*f);
}
Matrix2 operator*(float f, const Matrix2& m1){
  return Matrix2(m1.get(0, 0)*f, m1.get(1, 0)*f,
		 m1.get(0, 1)*f, m1.get(1, 1)*f);
}


float& Point3::operator[](int a){
  len = -1;
  return p[a];
}

float Point3::get(int a) const {
  return p[a];
}

Point3::Point3(){
  len = 0;
  x = 0; y = 0; z = 0;
}

Point3::Point3(float nx, float ny, float nz){
  len = -1;
  x = nx, y = ny, z = nz;
}

void Point3::normalize(){
  float invsq = 1/length();
  len = 1;
  x*=invsq; y*=invsq; z*=invsq;
}

float Point3::length(){
  return len>=0?len:(len=sqrt(x*x + y*y + z*z));
}

std::string Point3::str() const {
  std::ostringstream oss;
  oss<<"["<<p[0]<<", "<<p[1]<<", "<<p[2]<<"]";
  return oss.str();
}


float* Matrix3::operator[](int a) {
  return mat + 3*a;
}

float Matrix3::get(int a, int b) const {
  return mat[a + 3*b];
}

Matrix3::Matrix3(int i){
  switch(i){
  case FLATALG_MATRIX_IDENTITY:
  default:
    for(int i = 0; i < 9; i++){
      mat[i] = i%4 == 0;
    }
    break;
  }
    
}

Matrix3::Matrix3(int i, float theta){
  switch(i){
    float ct, st;
  case FLATALG_MATRIX_ROTATION:
  case FLATALG_MATRIX_ROTATION_Z:
    st = sin(theta), ct = cos(theta);
    mat[0] = ct; mat[1] = -st; mat[2] = 0;
    mat[3] = st; mat[4] = ct; mat[5] = 0;
    mat[6] = 0; mat[7] = 0; mat[8] = 1;
    break;
  case FLATALG_MATRIX_ROTATION_X:
    st = sin(theta), ct = cos(theta);
    mat[0] = 1; mat[1] = 0; mat[2] = 0;
    mat[3] = 0; mat[4] = ct; mat[5] = -st;
    mat[6] = 0; mat[7] = st; mat[8] = ct;
    break;
  case FLATALG_MATRIX_ROTATION_Y:
    st = sin(theta), ct = cos(theta);
    mat[0] = ct; mat[1] = 0; mat[2] = st;
    mat[3] = 0; mat[4] = 1; mat[5] = 0;
    mat[6] = -st; mat[7] = 0; mat[8] = ct;
    break;
  }
}

Matrix3::Matrix3(int i, float arg1, float arg2){
  switch(i){
  case FLATALG_MATRIX_ROTATION:
    float st = sin(arg1), ct = cos(arg1),
      sp = sin(arg2), cp = cos(arg2);
    mat[0] = ct; mat[1] = -st; mat[2] = 0;
    mat[3] = st*cp; mat[4] = ct*cp; mat[5] = -sp;
    mat[6] = st*sp; mat[7] = ct*sp; mat[8] = cp;
    break;
  }
}

Matrix3::Matrix3(int i, const Vector3& v, float arg1){
  switch(i){
  case FLATALG_MATRIX_ROTATION:
    float norm = sqrt(v.x*v.x + v.y*v.y  + v.z*v.z);
    Matrix3 what(0, -v.get(2), v.get(1),
		 v.get(2), 0, -v.get(0),
		 -v.get(1), v.get(0), 0);
    Matrix3 u = Matrix3(FLATALG_MATRIX_IDENTITY) + what/(norm)*sin(arg1) + what*what/pow(norm, 2)*(1 - cos(arg1));
    
    for(int i = 0 ; i< 9; i++){
      mat[i] = u.mat[i];
    }

    break;
  }
}

Matrix3::Matrix3(){
  for(int i = 0; i < 9; i++){
    mat[i] = i%4 == 0;
  }

}

Matrix3::Matrix3(float a1, float a2, float a3,
		 float a4, float a5, float a6,
		 float a7, float a8, float a9){
  mat[0] = a1; mat[1] = a2; mat[2] = a3;
  mat[3] = a4; mat[4] = a5; mat[5] = a6;
  mat[6] = a7; mat[7] = a8; mat[8] = a9;
}

std::string Matrix3::str() const {
  std::ostringstream oss;
  oss<<"["<<mat[0]<<", "<<mat[1]<<", "<<mat[2]<<"]"<<std::endl
     <<"["<<mat[3]<<", "<<mat[4]<<", "<<mat[5]<<"]"<<std::endl
     <<"["<<mat[6]<<", "<<mat[7]<<", "<<mat[8]<<"]";
  return oss.str();
}

float Matrix3::det() const {
  return mat[0]*(mat[4]*mat[8] - mat[5]*mat[7]) - 
         mat[1]*(mat[3]*mat[8] - mat[5]*mat[6]) +
	 mat[2]*(mat[3]*mat[7] - mat[4]*mat[6]);
}

Vector3 cross(const Vector3& v1, const Vector3& v2){
  return Vector3(v1.get(1)*v2.get(2) - v1.get(2)*v2.get(1),
		 v1.get(2)*v2.get(0) - v1.get(0)*v2.get(2),
		 v1.get(0)*v2.get(1) - v1.get(1)*v2.get(0));
}

float operator*(const Vector3& v1, const Vector3& v2){
  return v1.get(0)*v2.get(0) + v1.get(1)*v2.get(1) + v1.get(2)*v2.get(2);
}

Vector3 operator*(const Vector3& v, float f){
  return Vector3(v.get(0)*f, v.get(1)*f, v.get(2)*f);
}

Vector3 operator*(float f, const Vector3& v){
  return Vector3(v.get(0)*f, v.get(1)*f, v.get(2)*f);
}

Vector3 operator+(const Vector3& v1, const Vector3& v2){
  return Vector3(v1.get(0) + v2.get(0), v1.get(1) + v2.get(1), v1.get(2) + v2.get(2));
}

Vector3 operator-(const Vector3& v1, const Vector3& v2){
  return Vector3(v1.get(0) - v2.get(0), v1.get(1) - v2.get(1), v1.get(2) - v2.get(2));
}

Vector3 operator/(const Vector3& v, float f){
  return Vector3(v.get(0)/f, v.get(1)/f, v.get(2)/f);
}

Vector3 operator*(const Matrix3& m, const Vector3& v){
  return Vector3(m.get(0, 0)*v.get(0) + m.get(1, 0)*v.get(1) + m.get(2, 0)*v.get(2),
		 m.get(0, 1)*v.get(0) + m.get(1, 1)*v.get(1) + m.get(2, 1)*v.get(2),
		 m.get(0, 2)*v.get(0) + m.get(1, 2)*v.get(1) + m.get(2, 2)*v.get(2));
}

Vector3 operator*(const Vector3& v, const Matrix3& m){
  return Vector3(m.get(0, 0)*v.get(0) + m.get(0, 1)*v.get(1) + m.get(0, 2)*v.get(2),
		 m.get(1, 0)*v.get(0) + m.get(1, 1)*v.get(1) + m.get(1, 2)*v.get(2),
		 m.get(2, 0)*v.get(0) + m.get(2, 1)*v.get(1) + m.get(2, 2)*v.get(2));
}

Matrix3 operator~(const Matrix3& m){
  return Matrix3(m.get(0, 0), m.get(0, 1), m.get(0, 2),
		 m.get(1, 0), m.get(1, 1), m.get(1, 2),
		 m.get(2, 0), m.get(2, 1), m.get(2, 2));
}

Matrix3 operator+(const Matrix3& m1, const Matrix3& m2){
  return Matrix3(m1.get(0, 0) + m2.get(0, 0), m1.get(1, 0) + m2.get(1, 0), m1.get(2, 0) + m2.get(2, 0),
		 m1.get(0, 1) + m2.get(0, 1), m1.get(1, 1) + m2.get(1, 1), m1.get(2, 1) + m2.get(2, 1),
		 m1.get(0, 2) + m2.get(0, 2), m1.get(1, 2) + m2.get(1, 2), m1.get(2, 2) + m2.get(2, 2));
}
 
Matrix3 operator*(const Matrix3& m, float f){
  return Matrix3(m.get(0, 0)*f, m.get(1, 0)*f, m.get(2, 0)*f,
		 m.get(0, 1)*f, m.get(1, 1)*f, m.get(2, 1)*f,
		 m.get(0, 2)*f, m.get(1, 2)*f, m.get(2, 2)*f);
}

Matrix3 operator*(float f, const Matrix3& m){
  return Matrix3(m.get(0, 0)*f, m.get(1, 0)*f, m.get(2, 0)*f,
		 m.get(0, 1)*f, m.get(1, 1)*f, m.get(2, 1)*f,
		 m.get(0, 2)*f, m.get(1, 2)*f, m.get(2, 2)*f);
}

Matrix3 operator/(const Matrix3& m, float f){
  return Matrix3(m.get(0, 0)/f, m.get(1, 0)/f, m.get(2, 0)/f,
		 m.get(0, 1)/f, m.get(1, 1)/f, m.get(2, 1)/f,
		 m.get(0, 2)/f, m.get(1, 2)/f, m.get(2, 2)/f);
}

Matrix3 operator*(const Matrix3& m1, const Matrix3& m2){
  return Matrix3(m1.get(0, 0)*m2.get(0, 0) + m1.get(1, 0)*m2.get(0, 1) + m1.get(2, 0)*m2.get(0, 2),
		 m1.get(0, 0)*m2.get(1, 0) + m1.get(1, 0)*m2.get(1, 1) + m1.get(2, 0)*m2.get(1, 2),
		 m1.get(0, 0)*m2.get(2, 0) + m1.get(1, 0)*m2.get(2, 1) + m1.get(2, 0)*m2.get(2, 2),

		 m1.get(0, 1)*m2.get(0, 0) + m1.get(1, 1)*m2.get(0, 1) + m1.get(2, 1)*m2.get(0, 2),
		 m1.get(0, 1)*m2.get(1, 0) + m1.get(1, 1)*m2.get(1, 1) + m1.get(2, 1)*m2.get(1, 2),
		 m1.get(0, 1)*m2.get(2, 0) + m1.get(1, 1)*m2.get(2, 1) + m1.get(2, 1)*m2.get(2, 2),

		 m1.get(0, 2)*m2.get(0, 0) + m1.get(1, 2)*m2.get(0, 1) + m1.get(2, 2)*m2.get(0, 2),
		 m1.get(0, 2)*m2.get(1, 0) + m1.get(1, 2)*m2.get(1, 1) + m1.get(2, 2)*m2.get(1, 2),
		 m1.get(0, 2)*m2.get(2, 0) + m1.get(1, 2)*m2.get(2, 1) + m1.get(2, 2)*m2.get(2, 2));
}


float* Matrix4::operator[](int a){
  return mat + 4*a;
}

float Matrix4::get(int a, int b) const{
  return mat[a + 4*b];
}

float Matrix4::get(int a) const{
  return mat[a];
}

Matrix4::Matrix4(int i){
  switch(i){
  case FLATALG_MATRIX_IDENTITY:
  default:
    for(int i = 0; i < 16; i++){
      mat[i] = i%5 == 0;
    }
    break;
  }
}

Matrix4::Matrix4(int i, float theta){
  switch(i){
    float st, ct;
  case FLATALG_MATRIX_ROTATION:
  case FLATALG_MATRIX_ROTATION_Z:
    st = sin(theta), ct = cos(theta);
    mat[0] = ct; mat[1] = -st; mat[2] = 0; mat[3] = 0;
    mat[4] = st; mat[5] = ct; mat[6] = 0; mat[7] = 0;
    mat[8] = 0; mat[9] = 0; mat[10] = 1; mat[11] = 0;
    mat[12] = 0; mat[13] = 0; mat[14] = 0; mat[15] = 1;
    break;
  case FLATALG_MATRIX_ROTATION_X:
    st = sin(theta), ct = cos(theta);
    mat[0] = 1; mat[1] = 0; mat[2] = 0; mat[3] = 0;
    mat[4] = 0; mat[5] = ct; mat[6] = -st; mat[7] = 0;
    mat[8] = 0; mat[9] = st; mat[10] = ct; mat[11] = 0;
    mat[12] = 0; mat[13] = 0; mat[14] = 0; mat[15] = 1;
    break;
  case FLATALG_MATRIX_ROTATION_Y:
    st = sin(theta), ct = cos(theta);
    mat[0] = ct; mat[1] = 0; mat[2] = st; mat[3] = 0;
    mat[4] = 0; mat[5] = 1; mat[6] = 0; mat[7] = 0;
    mat[8] = -st; mat[9] = 0; mat[10] = ct; mat[11] = 0;
    mat[12] = 0; mat[13] = 0; mat[14] = 0; mat[15] = 1;
    break;
  }
}

Matrix4::Matrix4(int i, float arg1, float arg2){
  switch(i){
  case FLATALG_MATRIX_ROTATION:
    float st = sin(arg1), ct = cos(arg1),
      sp = sin(arg2), cp = cos(arg2);
    mat[0] = ct; mat[1] = -st; mat[2] = 0; mat[3] = 0;
    mat[4] = st*cp; mat[5] = ct*cp; mat[6] = -sp; mat[7] = 0;
    mat[8] = st*sp; mat[9] = ct*sp; mat[10] = cp; mat[11] = 0;
    mat[12] = 0; mat[13] = 0; mat[14] = 0; mat[15] = 1;
    break;
  }
}

Matrix4::Matrix4(int i, const Point3& p){
  switch(i){
  case FLATALG_MATRIX_TRANSLATION:
    for(int i = 0; i< 16; i++){
      mat[i] = i%5 ==0;
    }
    mat[3] = p.get(0);
    mat[7] = p.get(1);
    mat[11] = p.get(2);
    break;
  }
}

Matrix4::Matrix4(){
  for(int i = 0; i < 16; i++){
    mat[i] = i%5 == 0;
  }
}
Matrix4::Matrix4(float a1, float a2, float a3, float a4,
	float a5, float a6, float a7, float a8,
	float a9, float a10, float a11, float a12,
		 float a13, float a14, float a15, float a16){
  mat[0] = a1; mat[1] = a2; mat[2] = a3; mat[3] = a4;
  mat[4] = a5; mat[5] = a6; mat[6] = a7; mat[7] = a8;
  mat[8] = a9; mat[9] = a10; mat[10] = a11; mat[11] = a12;
  mat[12] = a13; mat[13] = a14; mat[14] = a15; mat[15] = a16; 
}

std::string Matrix4::str() const{
  std::ostringstream oss;
  oss<<"["<<mat[0]<<", "<<mat[1]<<", "<<mat[2]<<", "<<mat[3]<<"]"<<std::endl
     <<"["<<mat[4]<<", "<<mat[5]<<", "<<mat[6]<<", "<<mat[7]<<"]"<<std::endl
     <<"["<<mat[8]<<", "<<mat[9]<<", "<<mat[10]<<", "<<mat[11]<<"]"<<std::endl
    <<"["<<mat[12]<<", "<<mat[13]<<", "<<mat[14]<<", "<<mat[15]<<"]";
  return oss.str();
}

Vector3 operator*(const Matrix4& m, const Vector3& v){
  return Vector3(m.get(0, 0)*v.get(0) + m.get(1, 0)*v.get(1) + m.get(2, 0)*v.get(2) + m.get(3, 0),
		 m.get(0, 1)*v.get(0) + m.get(1, 1)*v.get(1) + m.get(2, 1)*v.get(2) + m.get(3, 1),
		 m.get(0, 2)*v.get(0) + m.get(1, 2)*v.get(1) + m.get(2, 2)*v.get(2) + m.get(3, 2));
}

Vector3 operator*(const Vector3& v, const Matrix4& m){
  return Vector3(m.get(0, 0)*v.get(0) + m.get(0, 1)*v.get(1) + m.get(0, 2)*v.get(2) + m.get(0, 3),
		 m.get(1, 0)*v.get(0) + m.get(1, 1)*v.get(1) + m.get(1, 2)*v.get(2) + m.get(1, 3),
		 m.get(2, 0)*v.get(0) + m.get(2, 1)*v.get(1) + m.get(2, 2)*v.get(2) + m.get(2, 3));
}

Matrix4 operator*(const Matrix4& m, float f){
  Matrix4 m2;
  for(int i = 0; i < 16; i++){
    m2.mat[i] = f*m.get(i);
  }
  return m2;
}

Matrix4 operator*(float f, const Matrix4& m){
  Matrix4 m2;
  for(int i = 0; i < 16; i++){
    m2.mat[i] = f*m.get(i);
  }
  return m2;
}

Matrix4 operator~(const Matrix4& m){
  Matrix4 m2;
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 4; j++){
      m2[i][j] = m.get(i, j);
    }
  }
  return m2;
}

Matrix4 operator*(const Matrix4& m1, const Matrix4& m2){
  Matrix4 r;
  for(int i = 0; i < 4; i++){
    r[i][i] = 0;
    for(int j = 0; j < 4; j++){
      for(int k = 0; k < 4; k++){
	r[i][j] += m1.get(k, i)*m2.get(j, k);
      }
    }
  }
  return r;
}


namespace flatalg{
  typedef GeneralMatrix Matrix;
  typedef GeneralVector Vector;
}


float& GeneralVector::operator[](int a){
  return vec[a];
}

int GeneralVector::getLength() const{
  return l;
}

int GeneralVector::getL() const{
  return l;
}


float GeneralVector::get(int a) const{
  return vec[a];
}

GeneralVector::GeneralVector(int ll){
  l = ll;
  vec = new float[l];
}

GeneralVector::GeneralVector(int ll, const float* p){
  l = ll;
  vec = new float[l];
  for(int i = 0; i<  l; i++){
    vec[i] = p[i];
  }
}

GeneralVector::GeneralVector(const GeneralVector& gv){
  l = gv.getLength();
  vec = new float[l];
  for(int i = 0; i<  l;i++){
    vec[i] = gv.get(i);
  }
}

GeneralVector::~GeneralVector(){
  delete[] vec;
}

std::string GeneralVector::str() const{
  std::ostringstream iss;
  iss<<"[";
  for(int i = 0; i< l - 1; i++){
    iss<<vec[i]<<", ";
  }
  iss<<vec[l - 1]<<"]";
  return iss.str();
}


float operator*(const GeneralVector& g1, const GeneralVector& g2){
  float f = 0;
  if(g1.getLength() == g2.getLength()){
    for(int i = 0; i < g1.getLength(); i++){
      f += g1.get(i)*g2.get(i);
    }
  }
  return f;
}



GeneralVector operator+(const GeneralVector& g1, const GeneralVector& g2){
  GeneralVector g(g1.getLength());
  if(g1.getLength() == g2.getLength()){
    for(int i = 0;i < g.getLength(); i++){
      g[i] = g1.get(i) + g2.get(i);
    }
  }else{
    for(int i= 0; i< g.getLength(); i++){
      g[i] = 0;
    }
  }
  return g;
}

GeneralVector operator-(const GeneralVector& g1, const GeneralVector& g2){
  GeneralVector g(g1.getLength());
  if(g1.getLength() == g2.getLength()){
    for(int i = 0; i < g.getLength(); i++){
      g[i] = g1.get(i) - g2.get(i);
    }
  }else{
    for(int i= 0; i< g.getLength(); i++){
      g[i] = 0;
    }
  }
  return g;
}

GeneralVector operator*(const GeneralVector& g1, float f){
  GeneralVector g(g1.getLength());
  for(int i = 0; i< g.getLength(); i++){
    g[i] = f*g1.get(i);
  }
  return g;
}

GeneralVector operator*(float f, const GeneralVector& g1){
  GeneralVector g(g1.getLength());
  for(int i = 0; i< g.getLength(); i++){
    g[i] = f*g1.get(i);
  }
  return g;
}

GeneralVector operator/(const GeneralVector& g1, float f){
  GeneralVector g(g1.getLength());
  for(int i = 0; i< g.getLength(); i++){
    g[i] = g1.get(i)/f;
  }
  return g;
}


float* GeneralMatrix::operator[](int a){
  return mat + w*a;
}

int GeneralMatrix::getHeight() const{
  return h;
}

int GeneralMatrix::getWidth() const{
  return w;
}

int GeneralMatrix::getW() const{
  return w;
}
int GeneralMatrix::getH() const{
  return h;
}

float GeneralMatrix::get(int a, int b) const{
  return mat[b*w + a];
}

float GeneralMatrix::get(int a) const{
  return mat[a];
}

float GeneralMatrix::det() const{
  if(w != h){
    return 0;
  }
  if(w == 2 && h ==2){
    return mat[0]*mat[3] - mat[1]*mat[2];
  }else{
    float sum = 0;
    int u = 1;
    for(int i = 0; i < w; i++){
      sum += u*mat[i]*_minor(i, 0)._det();
      u*=-1;
    }
    return sum;
  }
}

float GeneralMatrix::_det() const{
  if(w == 2 && h ==2){
    return mat[0]*mat[3] - mat[1]*mat[2];
  }else{
    float sum = 0;
    int u = 1;
    for(int i = 0; i < w; i++){
      sum += u*mat[i]*_minor(i, 0)._det();
      u*=-1;
    }
    return sum;
  }
}

GeneralMatrix GeneralMatrix::inv() const{
  GeneralMatrix g(w, h);
  for(int i= 0; i< h; i++){
    for(int j = 0; j < w; j++){
      g[i][j] = _minor(j, i).det();
    }
  }
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      g[i][j] = (i + j)&1?-g[i][j]:g[i][j];
    }
  }
  
  GeneralMatrix g2 = ~g;

  return g2/det();
}
GeneralMatrix::GeneralMatrix(int ww, int hh){
  w = ww; h = hh;
  mat = new float[w*h];
}
GeneralMatrix::GeneralMatrix(int ww, int hh, const float* p){
  w = ww; h = hh;
  mat = new float[w*h];
  for(int i = 0; i< w*h; i++){
    mat[i] = p[i];
  }
}

GeneralMatrix::GeneralMatrix(const GeneralMatrix& gm){
  w = gm.getW();
  h = gm.getH();
  mat = new float[w*h];
  for(int i = 0; i< h; i++){
    for(int j = 0; j < w; j++){
      mat[i*w + j] = gm.get(j, i);
    }
  }
}

GeneralMatrix::~GeneralMatrix(){
  delete[] mat;
}
  

GeneralMatrix GeneralMatrix::_minor(int a, int b) const{
  GeneralMatrix g(w -1, h - 1);
  int offi = 0;
  int offj = 0;
  for(int i = 0; i < h - 1; i++){
    if(i == b)
      offi++;
    for(int j = 0; j < w - 1; j++){
      if(j == a)
	offj++;

      g[i][j] = this->get(j + offj, i + offi);
    }
    offj = 0;
  }
  return g;
}


std::string GeneralMatrix::str() const{
  std::ostringstream oss;
  oss<<"[";
  for(int i =0; i < h -1; i++){
    oss<<"[";
    for(int j = 0; j < w - 1; j++){
      oss<<this->get(j, i)<<", ";
    }
    oss<<this->get(w - 1, i)<<"]"<<std::endl;
  }
  oss<<"[";
  for(int i = 0; i < w-1; i++){
    oss<<this->get(i, h - 1)<<", ";
  }
  oss<<this->get(w-1, h-1)<<"]]"<<std::endl;
  return oss.str();
}

GeneralVector operator*(const GeneralMatrix& gm, const GeneralVector& gv){
  GeneralVector g(gm.getH());
  if(gm.getW() != gv.getL()){
    return g;
  }else{
    for(int i = 0; i< gm.getH(); i++){
      g[i] = 0;
      for(int j = 0; j< gm.getW(); j++){
	g[i] += gm.get(j, i)*gv.get(j);
      }
    }
    return g;
  }
}

GeneralVector operator*(const GeneralVector& gv, const GeneralMatrix& gm){
  GeneralVector g(gm.getW());
  if(gm.getH() != gv.getL()){
    return g;
  }else{
    for(int i = 0; i< gm.getW(); i++){
      g[i] = 0;
      for(int j = 0; j< gm.getH(); j++){
	g[i] += gm.get(j, i)*gv.get(j);
      }
    }
    return g;
  }
}

GeneralMatrix operator~(const GeneralMatrix& gm){
  GeneralMatrix g(gm.getH(), gm.getW());
  for(int i= 0 ; i< gm.getH(); i++){
    for(int j = 0; j < gm.getW(); j++){
      g[j][i] = gm.get(j, i);
    }
  }
  return g;
}

GeneralMatrix operator*(const GeneralMatrix& gm, float f){
  GeneralMatrix g(gm.getW(), gm.getH());
  for(int i  = 0; i< gm.getH(); i++){
    for(int j = 0; j < gm.getW(); j++){
      g[i][j] = gm.get(j, i)*f;
    }
  }
  return g;
}

GeneralMatrix operator*(float f, const GeneralMatrix& gm){
  GeneralMatrix g(gm.getW(), gm.getH());
  for(int i  = 0; i< gm.getH(); i++){
    for(int j = 0; j < gm.getW(); j++){
      g[i][j] = gm.get(j, i)*f;
    }
  }
  return g;
}

GeneralMatrix operator/(const GeneralMatrix& gm, float f){
  GeneralMatrix g(gm.getW(), gm.getH());
  for(int i  = 0; i< gm.getH(); i++){
    for(int j = 0; j < gm.getW(); j++){
      g[i][j] = gm.get(j, i)/f;
    }
  }
  return g;
}
  
GeneralMatrix operator*(const GeneralMatrix& g1, const GeneralMatrix& g2){
  GeneralMatrix g(g2.getW(), g1.getH());
  if(g1.getW() == g2.getH()){
    for(int i = 0; i< g1.getH(); i++){
      for(int j = 0; j < g2.getW(); j++){
	g[i][j] = 0;
	for(int k = 0; k < g1.getW(); k++){
	  g[i][j] += g1.get(k, i)*g2.get(j, k);
	}
      }
    }
  }
  return g;
}

#endif // FLATALG_IMPLEMENTATION
