//FlatAlg.h - Haakon Flatval

#ifndef INCLUDED_FLATALG
#define INCLUDED_FLATALG

#define FLATALG_MATRIX_IDENTITY 0
//Rotations around: 1. Z-axis  2. X-axis 3. Y-axis
#define FLATALG_MATRIX_ROTATION 1
#define FLATALG_MATRIX_TRANSLATION 2
#define FLATALG_MATRIX_ROTATION_X 3
#define FLATALG_MATRIX_ROTATION_Y 4
#define FLATALG_MATRIX_ROTATION_Z 5

#include <string>

#define F_PI 3.141592653589793f

struct Point2{
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
  float length() const;
  float sqLength() const;

  std::string str() const;
};

typedef Point2 Vector2;

struct Matrix2{
  float mat[9];

  float* operator[](int a);
  Matrix2 inverse() const;
  float det() const;
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
  float length() const;
  float sqLength() const;

  Point3 normalized() const;

  std::string str() const;
};

typedef Point3 Vector3;

struct Matrix3{
  float mat[9];

  float* operator[](int a);

  float det() const;

  Matrix3 inverse() const;

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

Vector3 operator-(const Vector3& v);

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
  Matrix3 toMatrix3() const;
};

Vector3 operator*(const Matrix4& m, const Vector3& v);

Vector3 operator*(const Vector3& v, const Matrix4& m);

Matrix4 operator*(const Matrix4& m, float f);

Matrix4 operator*(float f, const Matrix4& m);

Matrix4 operator~(const Matrix4& m);

Matrix4 operator*(const Matrix4& m1, const Matrix4& m2);

namespace flatalg{
  Matrix4 lookAt(const Vector3& position,
		 const Vector3& target,
		 const Vector3& up);
  
  Matrix4 projection(float angle_radians,
		     float ratio,
		     float near,
		     float far);
};
 
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

namespace flatalg{
  
  typedef GeneralMatrix Matrix;
  typedef GeneralVector Vector;
}

#endif // INCLUDED_FLATALG

