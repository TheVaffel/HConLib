#include <FlatAlg.hpp>

// Contains specialized template functions (that cannot otherwise be included in an archive
namespace falg {
    template<>
    void Matrix<2, 2>::init_rotation(flatalg_t theta) {
        init_2x2_rotation(theta);
    }

    template<>
    void Matrix<3, 3>::init_rotation(int offset, flatalg_t theta) {
        init_3x3_rotation(offset, theta);
    }

    template<>
    void Matrix<4, 4>::init_rotation(int offset, flatalg_t theta) {
        init_3x3_rotation(offset, theta);
        for(int i = 0; i < 3; i++) {
            (*this)(3, i) = 0;
            (*this)(i, 3) = 0;
        }

        (*this)(3, 3) = 1;
    }

    template<>
    void Matrix<4, 4>::set3Dtrans(const Matrix<3, 1>& vec) {
        for(int i = 0; i < 3; i++) {
            (*this)(i, 3) = vec[i];
        }
    }

    template<>
    template<>
    void Matrix<4, 4>::init_args(FlatAlgMatrixFlag flag, const Matrix<3, 1>& vec) {
        switch(flag) {
        case FLATALG_MATRIX_TRANSLATION:
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    if( i == j) {
                        (*this)(i, j) = 1;
                    } else {
                        (*this)(i, j) = 0;
                    }
                }
            }
            set3Dtrans(vec);
            break;
        case FLATALG_MATRIX_SCALE:
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (i == j) {
                        (*this)(i, j) = 1;
                    } else if (i == 3 && j < 3) {
                        (*this)(i, j) = vec[j];
                    } else {
                        (*this)(i, j) = 0;
                    }
                }
            }
        default:
            throw std::invalid_argument("[FlatAlg::Matrix4] No constructor for that flag-parameter combination");
        }
    }

    template<>
    void Matrix<4, 4>::init_args(FlatAlgMatrixFlag flag, flatalg_t arg0) {
        switch(flag) {
        case FLATALG_MATRIX_SCALE:
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (i == j && i < 3) {
                        (*this)(i, j) = arg0;
                    } else if (i == j) {
                        (*this)(i, j) = 1;
                    } else {
                        (*this)(i, j) = 0;
                    }
                }
            }
            break;
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
            throw std::invalid_argument("[FlatAlg::Matrix4] No constructor for that flag-parameter combination");
        }
    }

    template<>
    template<>
    void Matrix<4, 4>::init_args(FlatAlgMatrixFlag flag, const Matrix<3, 3>& mat, const Matrix<3, 1>& vec) {
        switch(flag) {
        case FLATALG_MATRIX_TRANSFORM:
            this->set3x3(mat);
            this->set3Dtrans(vec);
            (*this)(3, 0) = 0; (*this)(3, 1) = 0; (*this)(3, 2) = 0; (*this)(3, 3) = 1;
            break;
        default:
            throw std::invalid_argument("[FlatAlg::Matrix4] No constructor for that flag-parameter combination");
        }
    }

    template<>
    void Matrix<4, 4>::setProjection(flatalg_t horizontalFOVRadians, flatalg_t invAspect, flatalg_t near, flatalg_t far) {
        flatalg_t fx = 1/tan(horizontalFOVRadians/2);
        flatalg_t fy = 1/(invAspect*tan(horizontalFOVRadians/2));

        for(int i = 0; i < 4 * 4; i++) {
            arr[i] = 0;
        }

        (*this)(0, 0) = fx;
        (*this)(1, 1) = fy;
        (*this)(2, 2) = -(far + near) / (far - near);
        (*this)(2, 3) = -(2 * far * near) / (far - near);
        (*this)(3, 2) = -1;

    }

    template<>
    void Matrix<4, 4>::init_args(FlatAlgMatrixFlag flag, flatalg_t arg0, flatalg_t arg1, flatalg_t arg2, flatalg_t arg3) {
        switch(flag) {
        case FLATALG_MATRIX_PROJECTION:
            this->setProjection(arg0, arg1, arg2, arg3);
            break;
        default:
            throw std::invalid_argument("[FlatAlg::Matrix4] No constructor for that flag-parameter combination");
        }
    }

    template<>
    void Matrix<3, 3>::setRotation(const Matrix<3, 1>& vec, flatalg_t arg0) {
        flatalg_t norm = vec.norm();
        Mat3 what(0, -vec.z(), vec.y(),
                  vec.z(), 0, -vec.x(),
                  -vec.y(), vec.x(), 0);
        Mat3 u = Mat3(FLATALG_MATRIX_IDENTITY) + what * (sin(arg0) / norm) + what * what * ((1 - cos(arg0)) /  (norm * norm));

        for(int i = 0; i < 3 * 3; i++) {
            this->arr[i] = u[i];
        }
    }


    template<>
    template<>
    void Matrix<3, 3>::init_args(FlatAlgMatrixFlag flag, const Matrix<3, 1>& vec, flatalg_t arg0) {
        switch(flag) {
        case FLATALG_MATRIX_ROTATION:
            this->setRotation(vec, arg0);
            break;
        default:
            throw std::invalid_argument("[FlatAlg::Matrix3] No constructor for that flag-parameter combination");
        }
    }

    template<>
    void Matrix<3, 3>::setRotation(flatalg_t arg0, flatalg_t arg1) {

        flatalg_t st = sin(arg0), ct = cos(arg0),
            sp = sin(arg1), cp = cos(arg1);
        arr[0] = ct; arr[1] = -st; arr[2] = 0;
        arr[3] = st*cp; arr[4] = ct*cp; arr[5] = -sp;
        arr[6] = st*sp; arr[7] = ct*sp; arr[8] = cp;
    }

    template<>
    void Matrix<3, 3>::init_args(FlatAlgMatrixFlag flag, flatalg_t arg0, flatalg_t arg1) {
        switch(flag){
        case FLATALG_MATRIX_ROTATION:
            setRotation(arg0, arg1);
            break;
        default:
            throw std::invalid_argument("[FlatAlg::Matrix3] No constructor for that flag-parameter combination");
        }
    }

    template<>
    void Matrix<4, 4>::setLookAt(const Matrix<3, 1>& pos, const Matrix<3, 1>& target, const Matrix<3, 1>& up) {
        Vec3 dir = (target - pos);
        Vec3 normDir = dir.normalized();
        Vec3 right = cross(normDir, up);
        if(right.sqNorm() > 1e-5){
            right = right.normalized();
        }else{
            right = Vec3(up.y(), up.z(), up.x());
        }

        Vec3 nUp = cross(right, normDir);

        (*this)(0, 0) = right.x();    (*this)(0, 1) = right.y();    (*this)(0, 2) = right.z();    (*this)(0, 3) = - dot(pos, right);
        (*this)(1, 0) = nUp.x();      (*this)(1, 1) = nUp.y();      (*this)(1, 2) = nUp.z();      (*this)(1, 3) = - dot(pos, nUp);
        (*this)(2, 0) = -normDir.x(); (*this)(2, 1) = -normDir.y(); (*this)(2, 2) = -normDir.z(); (*this)(2, 3) = dot(pos, normDir);
        (*this)(3, 0) = 0;            (*this)(3, 1) = 0;            (*this)(3, 2) = 0;            (*this)(3, 3) = 1;
    }

    template<>
    void Matrix<4, 4>::init_args(FlatAlgMatrixFlag flag, const Matrix<3, 1>& arg0, const Matrix<3, 1>& arg1, const Matrix<3, 1>& arg2) {
        switch(flag) {
        case FLATALG_MATRIX_LOOK_AT:
            this->setLookAt(arg0, arg1, arg2);
            break;
        default:
            throw std::invalid_argument("[FlatAlg::Matrix4] No constructor for that flag-parameter combination");
        }
    }

    flatalg_t cross(const Vec2& v1, const Vec2& v2) {
        return v1[0] * v2[1] - v1[1] * v2[0];
    }

    Vec3 cross(const Vec3& v1, const Vec3& v2) {
        return Vec3(v1[1] * v2[2] - v1[2] * v2[1],
                    - (v1[0] * v2[2] - v1[2] * v2[0]),
                    v1[0] * v2[1] - v1[1] * v2[0]);
    }

    Quaternion::Quaternion() {
        x = 0.0;
        y = 0.0;
        z = 0.0;
        w = 1.0f;
    }

    Quaternion::Quaternion(float nx, float ny, float nz, float nw) {
        x = nx, y = ny, z = nz, w = nw;
    }

    Quaternion::Quaternion(float theta, const Vec3& axis) {
        w = cos(theta / 2);
        float st = sin(theta / 2);
        x = axis[0] * st;
        y = axis[1] * st;
        z = axis[2] * st;
    }

    Quaternion::Quaternion(const Vec3& v) {
        x = v[0];
        y = v[1];
        z = v[2];
        w = 0;
    }

    float Quaternion::norm() const {
        return sqrt(x * x + y * y + z * z + w * w);
    }

    void Quaternion::normalize() {
        float inv = 1.f / norm();
        x *= inv;
        y *= inv;
        z *= inv;
        w *= inv;
    }

    Quaternion Quaternion::normalized() const {
        float inv = 1.f / norm();
        return Quaternion(x * inv,
                          y * inv,
                          z * inv,
                          w * inv);
    }

    Quaternion Quaternion::conjugate() const {
        return Quaternion(-x,
                          -y,
                          -z,
                          w);
    }

    Vec3 Quaternion::getVector() const {
        return Vec3(x, y, z);
    }

    float Quaternion::getReal() const {
        return w;
    }

    Vec3 Quaternion::rotate(const Vec3& v) const {
        return ((*this) * Quaternion(v) * conjugate()).getVector();
    }

    Mat3 Quaternion::toMatrix() const {
        return Mat3(w * w + x * x - y * y - z * z, 2 * (x * y - w * z),           2 * (x * z + w * y),
                    2 * (x * y + w * z),           w * w - x * x + y * y - z * z, 2 * (y * z - w * x),
                    2 * (x * z - w * y),           2 * (y * z + w * x),           w * w - x * x - y * y + z * z);
    }

    Quaternion::Quaternion(const Mat3& mat) {
        // Following Wikipedia's approach:
        // https://en.wikipedia.org/wiki/Rotation_matrix#Quaternion

        float trace = mat[0] + mat[4] + mat[8];
        if (trace > -0.99) {
            float r = sqrt(1 + trace);
            float s = 1 / (2 * r);

            this->w = 0.5f * r;
            this->x = (mat[7] - mat[5]) * s;
            this->y = (mat[2] - mat[6]) * s;
            this->z = (mat[3] - mat[1]) * s;
        } else {
            float r = sqrt(1 + mat[0] - mat[4] - mat[8]);
            float s = 1 / (2 * r);

            this->w = (mat[7] - mat[5]) * s;
            this->x = 0.5f * r;
            this->y = (mat[1] + mat[3]) * s;
            this->z = (mat[6] + mat[2]) * s;
        }
    }

    std::string Quaternion::str() const {
        std::ostringstream oss;
        oss<< "[x = " << p[0] << ", y = " << p[1] << ", z = " << p[2] << ", w = " << p[3] << "]";
        return oss.str();
    }

    Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
        return Quaternion(q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
                          q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
                          q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
                          q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z);
    }

    Quaternion operator+(const Quaternion& q1, const Quaternion& q2) {
        return Quaternion(q1.x + q2.x,
                          q1.y + q2.y,
                          q1.z + q2.z,
                          q1.w + q2.w);
    }

    Quaternion operator-(const Quaternion& q1, const Quaternion& q2) {
        return Quaternion(q1.x - q2.x,
                          q1.y - q2.y,
                          q1.z - q2.z,
                          q1.w - q2.w);
    }

    Quaternion operator*(const Quaternion& q1, float f) {
        return Quaternion(q1.x * f,
                          q1.y * f,
                          q1.z * f,
                          q1.w * f);
    }

    Quaternion operator*(float f, const Quaternion& q1) {
        return q1 * f;
    }

    Quaternion operator/(const Quaternion& q1, float f) {
        return q1 * (1.f / f);
    }

    DualQuaternion::DualQuaternion() {
        // Construct identity
        q1 = Quaternion(0.f, 0.f, 0.f, 1.f);
        q2 = Quaternion(0.f, 0.f, 0.f, 0.f);
    }

    DualQuaternion::DualQuaternion(const Quaternion& nq1, const Quaternion& nq2) {
        q1 = nq1;
        q2 = nq2;
    }

    DualQuaternion::DualQuaternion(const Vec3& v) {
        q1 = Quaternion(0.f, 0.f, 0.f, 1.f);
        q2 = Quaternion(v);
    }

    DualQuaternion::DualQuaternion(const Quaternion& q, const Vec3& v) {
        q1 = q;
        q2 = 0.5 * Quaternion(v) * q;
    }

    DualQuaternion::DualQuaternion(const Mat4& mat) {
        q1 = Quaternion(mat.submatrix<3, 3>(0, 0));
        Vec3 d = Vec3(mat[3], mat[7], mat[11]);
        q2 = 0.5 * Quaternion(d) * q1;
    }

    Vec3 DualQuaternion::getVector() const {
        return q2.getVector();
    }

    Vec3 DualQuaternion::transform(const Vec3& v) const {
        return ((*this) * DualQuaternion(v) * fullConjugate()).getVector();
    }

    DualQuaternion DualQuaternion::dualConjugate() const {
        return DualQuaternion(q1, -1.f * q2);
    }

    DualQuaternion DualQuaternion::conjugate() const {
        return DualQuaternion(q1.conjugate(), q2.conjugate());
    }

    DualQuaternion DualQuaternion::fullConjugate() const {
        return DualQuaternion(q1.conjugate(), Quaternion(q2.x, q2.y, q2.z, -q2.w));
    }

    Mat4 DualQuaternion::toMatrix() const {
        Mat3 rr = q1.toMatrix();
        Vec3 v = (2 * q2 * q1.conjugate()).getVector();
        return Mat4(FLATALG_MATRIX_TRANSFORM, rr, v);
    }

    void DualQuaternion::normalize() {
        float inv = 1.f / q1.norm();
        q1 = q1 * inv;
        q2 = q2 * inv;
    }

    DualQuaternion DualQuaternion::normalized() const {
        float inv = 1.f / q1.norm();
        return DualQuaternion(q1 * inv, q2 * inv);
    }

    DualQuaternion operator*(const DualQuaternion& q1, const DualQuaternion& q2) {
        return DualQuaternion(q1.q1 * q2.q1, q1.q2 * q2.q1 + q1.q1 * q2.q2);
    }

    DualQuaternion operator*(const DualQuaternion& q1, float f) {
        return DualQuaternion(q1.q1 * f, q1.q2 * f);
    }

    DualQuaternion operator*(float f, const DualQuaternion& q1) {
        return q1 * f;
    }

    DualQuaternion operator+(const DualQuaternion& q1, const DualQuaternion& q2) {
        return DualQuaternion(q1.q1 + q2.q1, q1.q2 + q2.q2);
    }

    DualQuaternion operator-(const DualQuaternion& q1, const DualQuaternion& q2) {
        return DualQuaternion(q1.q1 - q2.q1, q1.q2 - q2.q2);
    }

    DualQuaternion operator/(const DualQuaternion& q1, float f) {
        float inv = 1.f / f;
        return DualQuaternion(q1.q1 * inv, q1.q2 * inv);
    }

    std::string DualQuaternion::str() const {
        std::ostringstream oss;
        oss << "[x1 = " << q1.p[0] << ", y1 = " << q1.p[1] << ", z1 = " << q1.p[2] << ", w1 = " << q1.p[3] << "]";
        oss << "[x2 = " << q2.p[0] << ", y2 = " << q2.p[1] << ", z2 = " << q2.p[2] << ", w2 = " << q2.p[3] << "]";
        return oss.str();
    }

};
