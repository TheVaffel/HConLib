// FlatAlg.hpp - Haakon Flatval (new version, started 24.05.2019

#ifndef INCLUDED_FLATALG
#define INCLUDED_FLATALG

#include <string>
#include <stdexcept>
#include <sstream>
#include <array>

#ifdef __AVX__
#include <immintrin.h>
#else // __AVX__
#ifndef FLATALG_ALLOW_NO_AVX
#error "If you want to use FlatAlg without AVX instructions, define FLATALG_ALLOW_NO_AVX before including FlatAlg"
#endif // FLATALG_ALLOW_NO_AVX
#endif // __AVX__

namespace falg {
    enum FlatAlgMatrixFlag {
        FLATALG_MATRIX_IDENTITY = 0,
        //Rotations around: 1. Z-axis  2. X-axis 3. Y-axis
        FLATALG_MATRIX_ROTATION = 1,
        FLATALG_MATRIX_TRANSLATION = 2,
        FLATALG_MATRIX_ROTATION_X = 3,
        FLATALG_MATRIX_ROTATION_Y = 4,
        FLATALG_MATRIX_ROTATION_Z = 5,
        FLATALG_MATRIX_TRANSFORM = 6,
        FLATALG_MATRIX_SCALE = 7,
        FLATALG_MATRIX_PROJECTION = 8,
        FLATALG_MATRIX_LOOK_AT = 9,
        FLATALG_MATRIX_FROM_DATA = 10,
        FLATALG_MATRIX_DIRECT = 11,
    };

#define F_PI 3.141592653589793f

#ifndef FLATALG_REAL_TYPE
#define FLATALG_REAL_TYPE float
#endif // ndef FLATALG_REAL_TYPE

    typedef FLATALG_REAL_TYPE flatalg_t;

    template<int n, int m, int num>
    constexpr bool _assert_vector_length_name_access() {
        static_assert(m == 1 && n > num && n <= 4, "Cannot access element by name if vector too small or too long, or not a vector but a matrix");
        return true;
    }


    // Utilities for use with AVX instructions
#ifdef __AVX__
    // We really only support this for float 
    template<typename T = flatalg_t>
    static constexpr T _sum_m256(__m256 a) {
        static_assert(std::is_same<T, flatalg_t>::value, "_sum_m256 must be called with flatalg_t as template argument");
        __m256 zero = _mm256_setzero_ps();

        __m256 add0 = _mm256_hadd_ps(a, zero);
        __m256 add1 = _mm256_hadd_ps(add0, zero);

        T* vv = (T*)&add1;
        return vv[0] + vv[4];
    }

    template<typename T = flatalg_t>
    static constexpr T _sum_m128(__m128 a) {
        static_assert(std::is_same<T, flatalg_t>::value, "_sum_m128 must be called with flatalg_t as template argument");
        __m128 zero = _mm_setzero_ps();
        __m128 add = _mm_hadd_ps(a, zero);

        T* vv = (T*) &add;
        return vv[0] + vv[1];
    }
  
    static constexpr int per256 = sizeof(__m256) / sizeof(flatalg_t);
    constexpr int per128 = sizeof(__m128) / sizeof(flatalg_t);

    template<int c>
    static constexpr int num_m256 = c / per256;

    template<int c>
    constexpr int num_m128 = (c % per256) / per128;

    template<int c>
    constexpr int num_normal = c % per128;
#endif // __AVX__

    // Matrix - main class, represents both vectors and matrices
    template <int n, int m>
    class Matrix {

        flatalg_t arr[n * m];


        void init(flatalg_t* arr_pointer, const flatalg_t& f);

        template<typename... fl_args>
        void init(flatalg_t* arr_pointer, const flatalg_t& f, fl_args...);

        /* template<typename... fl_args>
           void checkAndInit(fl_args...); */

        void init_2x2_rotation(flatalg_t theta); // for Mat2
        void init_3x3_rotation(int coordinate_offset, flatalg_t theta); // for Mat3 and Mat4
        void init_rotation(flatalg_t theta);
        void init_rotation(int coordinate_offset, flatalg_t theta);
        void init_identity();

        void set3x3(const Matrix<3, 3>& mat);
        void set3Dtrans(const Matrix<3, 1>& vec);
        void setProjection(flatalg_t horizontalFOVRadians, flatalg_t invAspect, flatalg_t near, flatalg_t far);
        void setLookAt(const Matrix<3, 1>& pos, const Matrix<3, 1>& target, const Matrix<3, 1>& up);
        void setRotation(const Matrix<3, 1>& vec, flatalg_t angle);
        void setRotation(flatalg_t angle1, flatalg_t angle2);

        void init_args(FlatAlgMatrixFlag flag);
        void init_args(FlatAlgMatrixFlag flag, const flatalg_t* data);
        void init_args(FlatAlgMatrixFlag flag, flatalg_t arg1);
        void init_args(FlatAlgMatrixFlag flag, flatalg_t arg1, flatalg_t arg2);
        void init_args(FlatAlgMatrixFlag flag, flatalg_t arg1, flatalg_t arg2, flatalg_t arg3);

        template<typename... fl_args>
        void init_args(const flatalg_t& f, fl_args... args);

        template<int a, int b>
        void init_args(FlatAlgMatrixFlag, const Matrix<a, b>& mat, const Matrix<a, 1>& vec);

        template<int a>
        void init_args(FlatAlgMatrixFlag flag, const Matrix<a, 1>& vec);

        template<int a>
        void init_args(FlatAlgMatrixFlag flag, const Matrix<a, 1>& vec, flatalg_t arg1);

        void init_args(FlatAlgMatrixFlag flag, flatalg_t arg0, flatalg_t arg1, flatalg_t arg2, flatalg_t arg3);
        void init_args(FlatAlgMatrixFlag flag, const Matrix<3, 1>& arg0, const Matrix<3, 1>& arg1, const Matrix<3, 1>& arg2);
    public:

        // Guarantees zero entries
        Matrix();

        template<typename... fl_args>
        Matrix(fl_args... args);

        flatalg_t& x() { _assert_vector_length_name_access<n, m, 0>(); return arr[0];};
        flatalg_t& y() { _assert_vector_length_name_access<n, m, 1>(); return arr[1];};
        flatalg_t& z() { _assert_vector_length_name_access<n, m, 2>(); return arr[2];};
        flatalg_t& w() { _assert_vector_length_name_access<n, m, 3>(); return arr[3];};

        const flatalg_t& x() const { _assert_vector_length_name_access<n, m, 0>(); return arr[0];};
        const flatalg_t& y() const { _assert_vector_length_name_access<n, m, 1>(); return arr[1];};
        const flatalg_t& z() const { _assert_vector_length_name_access<n, m, 2>(); return arr[2];};
        const flatalg_t& w() const { _assert_vector_length_name_access<n, m, 3>(); return arr[3];};

        flatalg_t& operator[](int a);
        const flatalg_t& operator[](int a) const;

        flatalg_t& operator()(int a);
        const flatalg_t& operator()(int a) const;

        flatalg_t& operator()(int a, int b);
        const flatalg_t& operator()(int a, int b) const;

        // operator flatalg_t() { static_assert(n == 1 && m == 1); return this->arr[0];}

        flatalg_t norm() const;
        flatalg_t sqNorm() const;
        Matrix<n, m> normalized() const;
        std::string str() const;

        // Introduce new template parameters to hijack functions with better error messages
        // (Instead of just requiring a=n, b=m here)

        template<int a, int b>
        constexpr Matrix<n, m>& operator+=(const Matrix<a, b>& mat);

        template<int a, int b>
        constexpr Matrix<n, m>& operator-=(const Matrix<a, b>& mat);

        constexpr Matrix<n, m>& operator*=(const flatalg_t& f);
        constexpr Matrix<n, m>& operator/=(const flatalg_t& f);

        constexpr Matrix<n - 1, m - 1> minor_matrix(int a, int b) const;
        constexpr flatalg_t det() const;
        constexpr Matrix<n, m> inv() const;

        template<int w, int h>
        constexpr Matrix<h, w> submatrix(int y, int x) const;
    };

    template<int n, int m, int a, int b>
    constexpr Matrix<n, m> operator+(const Matrix<n, m>& m1, const Matrix<a, b>& m2);

    template<int n, int m, int a, int b>
    constexpr Matrix<n, m> operator-(const Matrix<n, m>& m1, const Matrix<a, b>& m2);

    template<int n, int m, int a, int b>
    constexpr Matrix<n, b> operator*(const Matrix<n, m>& m1, const Matrix<a, b>& m2);

    template<int n, int m>
    constexpr Matrix<n, m> operator*(const flatalg_t& f, const Matrix<n, m>& m1);

    template<int n, int m>
    constexpr Matrix<n, m> operator*(const Matrix<n, m>& m1, const flatalg_t& f);

    template<int n, int m>
    constexpr Matrix<n, m> operator/(const Matrix<n, m>& m1, const flatalg_t& f);

    template<int n, int m>
    constexpr Matrix<m, n> operator~(const Matrix<n, m>& m1);

    template<int n, int m>
    constexpr Matrix<n, m> operator-(const Matrix<n, m>& mat);

    template<int n, int m>
    constexpr flatalg_t operator*(const Matrix<n, 1>& m1, const Matrix<m, 1>& m2);

    template<int n, int m>
    constexpr flatalg_t dot(const Matrix<n, 1>& m1, const Matrix<m, 1>& m2);

    template<int n>
    constexpr Matrix<n - 1, 1> operator*(const Matrix<n, n>& mat, const Matrix<n - 1, 1> vec);

    // Convenient typedefs, making API similar to old version

    typedef Matrix<2, 1> Vec2;
    typedef Matrix<3, 1> Vec3;
    typedef Matrix<4, 1> Vec4;
    typedef Matrix<2, 2> Mat2;
    typedef Matrix<3, 3> Mat3;
    typedef Matrix<4, 4> Mat4;

    flatalg_t cross(const Vec2& v1, const Vec2& v2);
    Vec3 cross(const Vec3& v1, const Vec3& v2);



    struct Quaternion {
        union {
            struct {flatalg_t x, y, z, w;};
            flatalg_t p[4];
        };

        // Identity rotation
        Quaternion();

        Quaternion(float x, float y, float z, float w);

        // Rotation around axis, assumes axis is normalized
        Quaternion(float theta, const Vec3& axis);

        Quaternion(const Vec3& v);
        Quaternion(const Mat3& mat);

        float norm() const;
        void normalize();
        Quaternion normalized() const;
        Quaternion conjugate() const;

        Vec3 getVector() const;
        float getReal() const;
        Vec3 rotate(const Vec3&) const;
        Mat3 toMatrix() const;

        std::string str() const;
    };

    Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
    Quaternion operator+(const Quaternion& q1, const Quaternion& q2);
    Quaternion operator-(const Quaternion& q1, const Quaternion& q2);
    Quaternion operator*(const Quaternion& q1, float f);
    Quaternion operator*(float f, const Quaternion& q1);
    Quaternion operator/(const Quaternion& q1, float f);

    // These are an actual thing:
    struct DualQuaternion {
        Quaternion q1, q2;

        DualQuaternion();

        // Vector represented as dual quaternion (1 + ve)
        DualQuaternion(const Vec3& v);

        // Quaternion is rotation, Vector is translation
        DualQuaternion(const Quaternion& q1, const Vec3& v);

        // KNOW WHAT YOU ARE DOING WITH THIS ONE (use the one above if in doubt)
        DualQuaternion(const Quaternion& q1, const Quaternion& q2);

        DualQuaternion(const Mat4& mat);

        void normalize();
        DualQuaternion normalized() const;
        Vec3 transform(const Vec3& v) const;

        // q1 + q2e -> q1 - q2e
        DualQuaternion dualConjugate() const;

        // q1 + q2e -> ~q1 + ~q2e
        DualQuaternion conjugate() const;

        // q1 + q2e -> ~q1 - ~q2e
        DualQuaternion fullConjugate() const;

        Vec3 getVector() const;
        Mat4 toMatrix() const;

        std::string str() const;
    };

    DualQuaternion operator*(const DualQuaternion& q1, const DualQuaternion& q2);
    DualQuaternion operator*(const DualQuaternion& q, float f);
    DualQuaternion operator*(float f, const DualQuaternion& q);
    DualQuaternion operator+(const DualQuaternion& q1, const DualQuaternion& q2);
    DualQuaternion operator-(const DualQuaternion& q1, const DualQuaternion& q2);
    DualQuaternion operator/(const DualQuaternion& q1, float f);



    /*
     * ***********************************
     * Implementation begins here
     * ***********************************
     */
};

#ifndef FLATALG_NO_IMPLEMENTATION

#include <cmath>
#include <sstream> // For str()

namespace falg {
    template<int n, int m>
    flatalg_t& Matrix<n, m>::operator[](int a) {
        return this->arr[a];
    }

    template<int n, int m>
    const flatalg_t& Matrix<n, m>::operator[](int a) const {
        return this->arr[a];
    }

    template<int n, int m>
    flatalg_t& Matrix<n, m>::operator()(int a) {
        static_assert(m == 1 || n == 1, "(<index>) notation can only be used if matrix is vector");
        return this->arr[a];
    }

    template<int n, int m>
    const flatalg_t& Matrix<n, m>::operator()(int a) const {
        static_assert(m == 1 || n == 1, "(<index>) notation can only be used if matrix is vector");
        return this->arr[a];
    }


    template<int n, int m>
    flatalg_t& Matrix<n, m>::operator()(int a, int b) {
        return this->arr[a * m + b];
    }

    template<int n, int m>
    const flatalg_t& Matrix<n, m>::operator()(int a, int b) const {
        return this->arr[a * m + b];
    }

    template<int n, int m>
    flatalg_t Matrix<n, m>::norm() const {
        return sqrt(this->sqNorm());
    }

    template<int n, int m>
    flatalg_t Matrix<n, m>::sqNorm() const {
        flatalg_t sum = 0.0;
#ifdef __AVX__
        const flatalg_t* currp = this->arr;

        for (int i = 0; i < 2 * (num_m256<n * m> / 2); i++) {
            __m256 vals = _mm256_loadu_ps(currp);
            __m256 vals2 = _mm256_loadu_ps(currp + per256);
            __m256 mul1 = _mm256_mul_ps(vals, vals);
            __m256 mul2 = _mm256_mul_ps(vals2, vals2);
            __m256 addz = _mm256_add_ps(mul1, mul2);

            sum += _sum_m256(addz);
	
            /* __m256 zero = _mm256_setzero_ps();
            
               __m256 add2 = _mm256_hadd_ps(addz, zero);
               __m256 add3 = _mm256_hadd_ps(add2, zero);

               flatalg_t* vv = (flatalg_t*)&add3;
               sum += vv[0] + vv[4]; */

            currp += per256;
            currp += per256;
            i++;
        }

        if constexpr(num_m256<n * m> % 2 == 1) {

                // Experiments show that, rather consistently, 
                // SIMD operations for eight elements
                // did not give profitable advantages

                // (Which is weird, because we can get 
                // speedups for n = 4)
                __m256 vals = _mm256_loadu_ps(currp);
                __m256 mul = _mm256_mul_ps(vals, vals);
                sum += _sum_m256(mul);
	
                /* for (int j = 0; j < per256; j++) {
                   sum += currp[j] * currp[j];
                   } */
                currp += per256;
            }

        // This will have max one iteration
        for (int i = 0; i < num_m128<n * m>; i++) {
            __m128 vals = _mm_loadu_ps(currp);
            __m128 muls = _mm_mul_ps(vals, vals);

            /* flatalg_t* vv = (flatalg_t*)&muls;
            // Maybe _hadd again is better?
            for (int j = 0; j < 4; j++) {
            sum += vv[j];
            } */
            sum += _sum_m128(muls);
            currp += per128;
        }

        for (int i = 0; i < num_normal<n * m>; i++) {
            sum += (*currp) * (*currp);
            currp++;
        }

#else // __AVX__
        for(int i = 0; i < n * m; i++) {
            sum += this->arr[i] * this->arr[i];
        }
#endif // __AVX__

        return sum;
    }

    template<int n, int m>
    Matrix<n, m> Matrix<n, m>::normalized() const {
        return (*this) / this->norm();
    }

    template<int n, int m>
    std::string Matrix<n, m>::str() const {
        std::ostringstream oss;
        oss << '[';
        for(int i = 0; i < n - 1; i++) {
            for(int j = 0; j < m - 1; j++) {
                oss << (*this)(i, j) << ", ";
            }
            oss << (*this)(i, m - 1) << ";\n";
        }

        for(int j = 0; j < m - 1; j++) {
            oss << (*this)(n - 1, j) << ", ";
        }

        oss << (*this)(n - 1, m - 1) << "]\n";

        return oss.str();
    }

    template<int n, int m>
    void Matrix<n, m>::init(flatalg_t* pointer, const flatalg_t& flalg) {
        (*pointer) = flalg;
    }

    template<int n, int m>
    template<typename ... fl_args>
    void Matrix<n, m>::init(flatalg_t* pointer, const flatalg_t& flalg, fl_args... args) {
        *(pointer++) = flalg;
        init(pointer, args...);
    }

    // Public


    template<int n, int m>
    Matrix<n, m>::Matrix() {
        static_assert(n > 0 && m > 0, "Matrix dimensions must be positive");
        for(int i = 0; i < n * m; i++) {
            arr[i] = static_cast<flatalg_t>(0);
        }
    }

    template<int n, int m>
    void Matrix<n, m>::init_identity() {
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < m; j++) {
                (*this)(i, j) = (j == i) ? 1 : 0;
            }
        }
    }

    template<int n, int m>
    void Matrix<n, m>::init_args(FlatAlgMatrixFlag flag) {
        switch(flag) {
        case FLATALG_MATRIX_IDENTITY:
            init_identity();
            break;
        default:
            throw std::invalid_argument("No constructor for that flag-parameter combination");
        }
    }


    template<int n, int m>
    template<typename... fl_args>
    void Matrix<n, m>::init_args(const flatalg_t& f, fl_args... args) {
        static_assert(sizeof...(fl_args) == n * m - 1, "[FlatAlg::Matrix] Matrix constructor did not take enough arguments. Maybe you used a float when a Matrix was expected?");
        init(this->arr, f, args...);
    }


    template<int n, int m>
    template<typename... fl_args>
    Matrix<n, m>::Matrix(fl_args... args) {
        static_assert(n > 0 && m > 0, "Matrix dimensions must be positive");

        init_args(args...);

    }

    template<int n, int m>
    void Matrix<n, m>::init_args(FlatAlgMatrixFlag flag, const flatalg_t* f) {
        static_assert(n > 0 && m > 0, "Matrix dimensions must be positive");
        switch(flag) {

        case FLATALG_MATRIX_FROM_DATA:
            for(int i = 0; i < n * m; i++) {
                arr[i] = f[i];
            }
            break;
        default:
            throw std::invalid_argument("No constructor for that flag-parameter combination");
        }
    }

    template<int n, int m>
    void Matrix<n, m>::init_2x2_rotation(flatalg_t arg0) {
        static_assert((n == 2 && m == 2), "Input to init_2x2_rotation must be 2x2 matrices");

        flatalg_t st = sin(arg0);
        flatalg_t ct = cos(arg0);
        (*this)(0, 0) = ct; (*this)(0, 1) = -st;
        (*this)(1, 0) = st; (*this)(1, 1) = ct;
    }

    template<int n, int m>
    void Matrix<n, m>::init_3x3_rotation(int offset, flatalg_t theta) {
        static_assert((n == 3 && m == 3) || (n == 4 && m == 4), "Input to init_3x3 rotation must be 3x3 or 4x4 matrices");

        flatalg_t st, ct;
        st = sin(theta), ct = cos(theta);

        switch(offset) {
        case 0:
            (*this)(0, 0) = ct; (*this)(0, 1) = -st; (*this)(0, 2) = 0;
            (*this)(1, 0) = st; (*this)(1, 1) = ct;  (*this)(1, 2) = 0;
            (*this)(2, 0) = 0;  (*this)(2, 1) = 0; (*this)(2, 2) = 1;

            break;
        case 1:
            (*this)(0, 0) = 1; (*this)(0, 1) = 0;  (*this)(0, 2) = 0;
            (*this)(1, 0) = 0; (*this)(1, 1) = ct; (*this)(1, 2) = -st;
            (*this)(2, 0) = 0; (*this)(2, 1) = st; (*this)(2, 2) = ct;

            break;
        case 2:
            (*this)(0, 0) = ct;  (*this)(0, 1) = 0; (*this)(0, 2) = st;
            (*this)(1, 0) = 0;   (*this)(1, 1) = 1; (*this)(1, 2) = 0;
            (*this)(2, 0) = -st; (*this)(2, 1) = 0; (*this)(2, 2) = ct;

            break;
        }
    }



    template<int n, int m>
    void Matrix<n, m>::init_args(FlatAlgMatrixFlag flag, flatalg_t arg0) {
        switch(flag) {
        case FLATALG_MATRIX_ROTATION:
        case FLATALG_MATRIX_ROTATION_Z:
            this->init_rotation(0, arg0);
            break;
        case FLATALG_MATRIX_ROTATION_X:
            this->init_rotation(1, arg0);
            break;
        case FLATALG_MATRIX_ROTATION_Y:
            this->init_rotation(2, arg0);
            break;
        default:
            throw std::invalid_argument("No constructor for that flag-parameter combination");
        }
    }

    template<int n, int m>
    void Matrix<n, m>::set3x3(const Matrix<3, 3>& mat) {
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                (*this)(i, j) = mat(i, j);
            }
        }
    }



    template<int n, int m>
    template<int a, int b>
    constexpr Matrix<n, m>& Matrix<n, m>::operator+=(const Matrix<a, b>& mat) {
        static_assert(n == a && m == b, "Matrix dimensions must match in additive operations!");

#ifdef __AVX__
        flatalg_t* currp = this->arr;
        const flatalg_t* currpb = mat.arr;

        for (int i = 0; i < num_m256<n * m>; i++) {
            __m256 a0 = _mm256_loadu_ps(currp);
            __m256 b0 = _mm256_loadu_ps(currpb);

            __m256 res = _mm256_add_ps(a0, b0);
            _mm256_storeu_ps(currp, res);

            currp += per256;
            currpb += per256;
        }

        for (int i = 0; i < num_m128<n * m>; i++) {
            __m128 a0 = _mm_loadu_ps(currp);
            __m128 b0 = _mm_loadu_ps(currpb);
        
            __m128 res = _mm_add_ps(a0, b0);
            _mm_storeu_ps(currp, res);

            currp += per128;
            currpb += per128;
        }

        for (int i = 0; i < num_normal<n * m>; i++) {
            *currp = *currp + *currpb;

            currp++;
            currpb++;
        }
#else // __AVX__
    
        for(int i = 0; i < n * m; i++) {
            this->arr[i] += mat[i];
        }
#endif // __AVX__

        return *this;
    }

    template<int n, int m>
    template<int a, int b>
    constexpr Matrix<n, m>& Matrix<n, m>::operator-=(const Matrix<a, b>& mat) {
        static_assert(n == a && m == b, "Matrix dimensions must match in additive operations!");

#ifdef __AVX__
        // Actually just a copy-paste from above (with three changes).. Oh well
        flatalg_t* currp = this->arr;
        const flatalg_t* currpb = mat.arr;

        for (int i = 0; i < num_m256<n * m>; i++) {
            __m256 a0 = _mm256_loadu_ps(currp);
            __m256 b0 = _mm256_loadu_ps(currpb);

            __m256 res = _mm256_sub_ps(a0, b0);
            _mm256_storeu_ps(currp, res);

            currp += per256;
            currpb += per256;
        }

        for (int i = 0; i < num_m128<n * m>; i++) {
            __m128 a0 = _mm_loadu_ps(currp);
            __m128 b0 = _mm_loadu_ps(currpb);

            __m128 res = _mm_sub_ps(a0, b0);
            _mm_storeu_ps(currp, res);

            currp += per128;
            currpb += per128;
        }

        for (int i = 0; i < num_normal<n * m>; i++) {
            *currp = *currp - *currpb;

            currp++;
            currpb++;
        }

#else // __AVX__

        for(int i = 0; i < n * m; i++) {
            this->arr[i] -= mat[i];
        }

#endif // __AVX__

        return *this;
    }

    template<int n, int m>
    constexpr Matrix<n, m>& Matrix<n, m>::operator*=(const flatalg_t& f) {

#ifdef __AVX__
        flatalg_t* currp = this->arr;
      
        __m256 mf = _mm256_set1_ps(f);
        for (int i = 0; i < num_m256<n * m>; i++) {
            __m256 a = _mm256_loadu_ps(currp);
            __m256 rr = _mm256_mul_ps(a, mf);

            _mm256_storeu_ps(currp, rr);

            currp += per256;
        }

        __m128 mmf = _mm_set1_ps(f);
        for (int i = 0; i < num_m128<n * m>; i++) {
            __m128 a = _mm_loadu_ps(currp);
            __m128 rr = _mm_mul_ps(a, mmf);

            _mm_storeu_ps(currp, rr);

            currp += per128;
        }

        for (int i = 0; i < num_normal<n * m>; i++) {
            *currp *= f;
            currp++;
        }

#else // __AVX__
        for(int i = 0; i < n * m; i++) {
            this->arr[i] *= f;
        }
#endif // __AVX__

        return *this;
    }

    template<int n, int m>
    constexpr Matrix<n, m>& Matrix<n, m>::operator/=(const flatalg_t& f) {
        flatalg_t f_inv = static_cast<flatalg_t>(1) / f;
        return (*this) *= f_inv;
    }

    template<int n, int m>
    constexpr Matrix<n, m> operator*(const flatalg_t& f, const Matrix<n, m>& mat) {
        Matrix<n, m> nmat;

#if __AVX__
        const flatalg_t* currp = &mat[0];
        flatalg_t* currr = &nmat[0];

        __m256 mf = _mm256_set1_ps(f);
        for (int i = 0; i < num_m256<n * m>; i++) {
            __m256 a = _mm256_loadu_ps(currp);
            __m256 rr = _mm256_mul_ps(a, mf);

            _mm256_storeu_ps(currr, rr);

            currp += per256;
            currr += per256;
        }

        __m128 mmf = _mm_set1_ps(f);
        for (int i = 0; i < num_m128<n * m>; i++) {
            __m128 a = _mm_loadu_ps(currp);
            __m128 rr = _mm_mul_ps(a, mmf);

            _mm_storeu_ps(currr, rr);

            currp += per128;
            currr += per128;
        }

        for (int i = 0; i < num_normal<n * m>; i++) {
            *currr = *currp * f;

            currp++;
            currr++;
        }
#else // __AVX__

        for(int i = 0; i < n * m; i++) {
            nmat[i] = mat[i] * f;
        } 
#endif // __AVX__
      
        return nmat;
    }

    template<int n, int m>
    constexpr Matrix<n, m> operator*(const Matrix<n, m>& mat, const flatalg_t& f) {
        return f * mat;
    }

    template<int n, int m>
    constexpr Matrix<n, m> operator/(const Matrix<n, m>& mat, const flatalg_t& f) {
        return mat * (1.0 / f);
    }


    template<int n, int m>
    constexpr Matrix<n - 1, m - 1> Matrix<n, m>::minor_matrix(int a, int b) const {
        static_assert(n > 1 && m > 1, "Matrix must be more than a single number to take minor");
        Matrix<(n > 1 ? n - 1 : 0), (m > 1 ? m - 1 : 0)> mat;

        int i = 0;
        int curri = 0;
        while(true) {
            if(i == a) {
                i++;
            }

            if(i >= n) {
                break;
            }

            int currj = 0;
            int j = 0;
            while(true) {
                if (j == b) {
                    j++;
                }

                if(j >= m) {
                    break;
                }

                mat(curri, currj) = (*this)(i, j);
                j++;
                currj++;
            }
            i++;
            curri++;
        }

        return mat;
    }

    template<int n, int m>
    constexpr flatalg_t Matrix<n, m>::det() const {
        static_assert(n == m, "Matrix must be quadratic in order for determinants to make sense");
        if constexpr(n == 1 && m == 1) {
                return (*this)(0, 0);
            } else if constexpr(n == 2 && m == 2) {
                flatalg_t dd = (*this)(0, 0) * (*this)(1, 1)  - (*this)(1, 0) * (*this)(0, 1);
                return dd;
            } else {
            flatalg_t sum = 0;
            for(int i = 0; i < m; i++) {
                flatalg_t ff = (*this)(0, i) * ((i & 1) ? -1 : 1) * this->minor_matrix(0, i).det();
                sum += ff;
            }
            return sum;
        }
    }

    template<int n, int m>
    constexpr Matrix<n, m> Matrix<n, m>::inv() const {
        static_assert(n == m, "Matrices must be square to be inverted");

        Matrix<n, m> g;
        for(int i= 0; i < n; i++){
            for(int j = 0; j < m; j++){
                g(i, j) = this->minor_matrix(i, j).det();
            }
        }

        for(int i = 0; i < n; i++){
            for(int j = 0; j < m; j++){
                g(i, j) = (i + j)&1?-g(i, j):g(i, j);
            }
        }

        Matrix<n, m> g2 = ~g;
        return g2 / this->det();
    }

    template<int n, int m>
    template<int w, int h>
    constexpr Matrix<h, w> Matrix<n, m>::submatrix(int y, int x) const {
        Matrix<h, w> res;
        for(int i = 0; i < h; i++) {
            for(int j = 0; j < w; j++) {
                res(i, j) = (*this)(i + y, j + x);
            }
        }

        return res;
    }

    template<int n, int m, int a, int b>
    constexpr Matrix<n, m> operator+(const Matrix<n, m>& m1, const Matrix<a, b>& m2) {
        static_assert(n == a && m == b, "Matrix dimensions must match in additive operations!");
        Matrix<n, m> mat;

#ifdef __AVX__
        const flatalg_t* currpa = &m1[0];
        const flatalg_t* currpb = &m2[0];
        flatalg_t* currpr = &mat[0];

    
        for(int i = 0; i < num_m256<n * m>; i++) {
            __m256 aa = _mm256_loadu_ps(currpa);
            __m256 bb = _mm256_loadu_ps(currpb);

            __m256 res = _mm256_add_ps(aa, bb);

            _mm256_storeu_ps(currpr, res);

            currpa += per256;
            currpb += per256;
            currpr += per256;
        }

        for(int i = 0; i < num_m128<n * m>; i++) {
            __m128 aa = _mm_loadu_ps(currpa);
            __m128 bb = _mm_loadu_ps(currpb);

            __m128 res = _mm_add_ps(aa, bb);

            _mm_storeu_ps(currpr, res);

            currpa += per128;
            currpb += per128;
            currpr += per128;
        }

        for(int i = 0; i < num_normal<n * m>; i++) {
            *currpr = *currpa + *currpb;

            currpr++;
            currpa++;
            currpb++;
        }
#else // __AVX__
        for(int i = 0; i < n*m; i++) {
            mat[i] = m1[i] + m2[i];
        }
#endif // __AVX__

        return mat;
    }

    template<int n, int m, int a, int b>
    constexpr Matrix<n, m> operator-(const Matrix<n, m>& m1, const Matrix<a, b>& m2) {
        static_assert(n == a && m == b, "Matrix dimensions must match in additive operations!");
        Matrix<n, m> mat;

#ifdef __AVX__
        const flatalg_t* currpa = &m1[0];
        const flatalg_t* currpb = &m2[0];
        flatalg_t* currpr = &mat[0];

        for(int i = 0; i < num_m256<n * m>; i++) {
            __m256 aa = _mm256_loadu_ps(currpa);
            __m256 bb = _mm256_loadu_ps(currpb);

            __m256 res = _mm256_sub_ps(aa, bb);

            _mm256_storeu_ps(currpr, res);

            currpa += per256;
            currpb += per256;
            currpr += per256;
        }

        for(int i = 0; i < num_m128<n * m>; i++) {
            __m128 aa = _mm_loadu_ps(currpa);
            __m128 bb = _mm_loadu_ps(currpb);

            __m128 res = _mm_sub_ps(aa, bb);

            _mm_storeu_ps(currpr, res);

            currpa += per128;
            currpb += per128;
            currpr += per128;
        }

        for(int i = 0; i < num_normal<n * m>; i++) {
            *currpr = *currpa - *currpb;

            currpr++;
            currpa++;
            currpb++;
        }
#else // __AVX__
        for(int i = 0; i < n*m; i++) {
            mat[i] = m1[i] - m2[i];
        }
#endif // __AVX__

        return mat;
    
    }

    template<int n, int m, int a, int b>
    constexpr Matrix<n, b> operator*(const Matrix<n, m>& m1, const Matrix<a, b>& m2) {
        static_assert(m == a, "Inner matrix dimensions must match in matrix product!");

        Matrix<n, b> mat;

#ifdef __AVX__
        for(int i = 0; i < b; i++) {
            int curr_row = 0;
            for(int j = 0; j < num_m256<m>; j++) {
                __m256 vals = _mm256_setr_ps(m2(curr_row + 0, i), m2(curr_row + 1, i),
                                             m2(curr_row + 2, i), m2(curr_row + 3, i),
                                             m2(curr_row + 4, i), m2(curr_row + 5, i),
                                             m2(curr_row + 6, i), m2(curr_row + 7, i));
                for(int k = 0; k < n; k++) {
                    __m256 other_vals = _mm256_loadu_ps(&m1(k, curr_row));
                    __m256 mul = _mm256_mul_ps(other_vals, vals);
                    mat(k, i) += _sum_m256(mul);
                }
                curr_row += per256;
            }

            for(int j = 0; j < num_m128<m>; j++) {
                __m128 vals = _mm_setr_ps(m2(curr_row + 0, i), m2(curr_row + 1, i),
                                          m2(curr_row + 2, i), m2(curr_row + 3, i));

                for(int k = 0; k < n; k++) {
                    __m128 other_vals = _mm_loadu_ps(&m1(k, curr_row));
                    __m128 mul = _mm_mul_ps(other_vals, vals);
                    mat(k, i) += _sum_m128(mul);
                }
                curr_row += per128;
            }

      
            for(int j = 0; j < num_normal<m>; j++) {
                for(int k = 0; k < n; k++) {
                    mat(k, i) += m2(curr_row, i) * m1(k, curr_row);
                }
                curr_row++;
            }
        }
#else // __AVX__

	 
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < b; j++) {
                for(int k = 0; k < m; k++) {
                    mat(i, j) += m1(i, k) * m2(k, j);
                }
            }
        }
#endif // __AVX__

        return mat;
    }

    template<int n, int m>
    constexpr Matrix<m, n> operator~(const Matrix<n, m>& m1) {
        Matrix<m, n> mat;
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < m; j++) {
                mat(j, i) = m1(i, j);
            }
        }

        return mat;
    }

    template<int n, int m>
    constexpr Matrix<n, m> operator-(const Matrix<n, m>& m1) {
        Matrix<n, m> mat;
        for(int i = 0; i < n * m; i++) {
            mat[i] = -m1[i];
        }

        return mat;
    }

    template<int n, int m>
    constexpr flatalg_t operator*(const Matrix<n, 1>& m1, const Matrix<m, 1>& m2) {
        static_assert(n == m, "Vectors in dot product must have the same dimension");
        flatalg_t sum = static_cast<flatalg_t>(0);
        for(int i = 0; i < n ; i++) {
            sum += m1[i] * m2[i];
        }

        return sum;
    }

    template<int n, int m>
    constexpr flatalg_t dot(const Matrix<n, 1>& m1, const Matrix<m, 1>& m2) {
        static_assert(n == m, "Vectors in dot product must have the same dimension");
        flatalg_t sum = 0;
        for(int i = 0; i < n; i++) {
            sum += m1[i] * m2[i];
        }

        return sum;
    }

    template<int n>
    constexpr Matrix<n - 1, 1> operator*(const Matrix<n, n>& mat, const Matrix<n - 1, 1> vec) {
        Matrix<n - 1, 1> res;
        for(int i = 0; i < n - 1; i++) {
            for(int j = 0; j < n - 1; j++) {
                res[i] += mat(i, j) * vec(j);
            }
        }

        for(int i = 0; i < n - 1; i++) {
            res[i] += mat(i, n - 1);
        }

        return res;
    }


#endif // ndef FLATALG_NO_IMPLEMENTATION

};
  
#endif // ndef INCLUDED_FLATALG
