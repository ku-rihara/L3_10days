#include "Quaternion.h"
#include "Vector3.h"
#include <cfloat>
#include <cmath>
#include <numbers>
#include <algorithm>


Quaternion Quaternion::operator-() const {
    return Quaternion(-x, -y, -z, -w);
}


Quaternion Quaternion::operator*(const Quaternion& obj) const {
    return Quaternion(
        w * obj.x + x * obj.w + y * obj.z - z * obj.y,
        w * obj.y - x * obj.z + y * obj.w + z * obj.x,
        w * obj.z + x * obj.y - y * obj.x + z * obj.w,
        w * obj.w - x * obj.x - y * obj.y - z * obj.z);
}

Quaternion Quaternion::operator*(float scalar) const {
    Quaternion result;

    result.w = this->w * scalar;
    result.x = this->x * scalar;
    result.y = this->y * scalar;
    result.z = this->z * scalar;

    return result;
}


Quaternion operator*(float scalar, const Quaternion& q) {
    return Quaternion(q.x * scalar, q.y * scalar, q.z * scalar, q.w * scalar);
}

Quaternion Quaternion::operator+(const Quaternion& obj) const {
    return Quaternion(
        x + obj.x,
        y + obj.y,
        z + obj.z,
        w + obj.w);
}

Quaternion Quaternion::Identity() {
    return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Quaternion::Conjugate() const {
    return Quaternion(-x, -y, -z, w);
}

float Quaternion::Norm() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}


Quaternion Quaternion::Normalize() const {
    float norm = Norm();
    if (norm == 0) {
        return Identity();
    }
    return Quaternion(x / norm, y / norm, z / norm, w / norm);
}

Quaternion Quaternion::Inverse() const {
    float norm = Norm();
    if (norm == 0) {
        return Identity();
    }
    Quaternion conjugate = Conjugate();
    float normSquared    = norm * norm;
    return Quaternion(conjugate.x / normSquared, conjugate.y / normSquared, conjugate.z / normSquared, conjugate.w / normSquared);
}


Quaternion Quaternion::MakeRotateAxisAngle(const Vector3& axis, const float& angle) {
    // 正規化された軸を使用
    Vector3 normalizedAxis = (axis).Normalize();
    float halfAngle        = angle / 2.0f;
    float sinHalfAngle     = std::sin(halfAngle);

    return Quaternion(
        normalizedAxis.x * sinHalfAngle,
        normalizedAxis.y * sinHalfAngle,
        normalizedAxis.z * sinHalfAngle,
        std::cos(halfAngle));
}

Vector3 Quaternion::RotateVector(const Vector3& vector) {
    Quaternion vectorQuat(vector.x, vector.y, vector.z, 0.0f);
    Quaternion inverseQuat = this->Inverse();

    // 回転の計算
    Quaternion rotatedQuat = (*this) * vectorQuat * inverseQuat;

    return Vector3(rotatedQuat.x, rotatedQuat.y, rotatedQuat.z);
}


float Quaternion::Dot(const Quaternion& q1, const Quaternion& q2) {
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}


Quaternion Quaternion::Lerp(const Quaternion& start, const Quaternion& end, const float& t) {
 
    Quaternion result = (start * (1.0f - t)) + (end * t);
    return (result).Normalize();
}


Quaternion Quaternion::Slerp(const Quaternion& start, Quaternion end, const float& t) {

    float dot = Quaternion::Dot(start, end);

    // 内積が負の場合、最短経路を取るためにEndを反転
    if (dot < 0.0f) {
        end = end * -1.0f;
        dot = -dot;
    }

    const float DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        return Lerp(start, end, t); 
    }

    float theta_0 = std::acos(dot);
    float theta   = theta_0 * t;

    float sin_theta   = std::sin(theta);
    float sin_theta_0 = std::sin(theta_0);

    float s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
    float s1 = sin_theta / sin_theta_0;

    return (start * s0) + (end * s1);
}

Quaternion Quaternion::EulerToQuaternion(const Vector3& Euler) {
    Quaternion result;
    // オイラー角からクォータニオンを生成
    Quaternion qx = Quaternion::MakeRotateAxisAngle(Vector3(1, 0, 0), Euler.x);
    Quaternion qy = Quaternion::MakeRotateAxisAngle(Vector3(0, 1, 0), Euler.y);
    Quaternion qz = Quaternion::MakeRotateAxisAngle(Vector3(0, 0, 1), Euler.z);

    // XYZ順で回転を合成
    result      = qx * qy * qz;
    result = result.Normalize();

    return result;
}

float Quaternion::GetRollFromQuaternion() {
    // クォータニオンからオイラー角のロール成分を抽出
    float sinR_cosp = 2 * (w * x + y * z);
    float cosR_cosp = 1 - 2 * (x * x + y * y);
    return std::atan2(sinR_cosp, cosR_cosp);
}

Quaternion Quaternion::LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 forward = (target - eye).Normalize();
    Vector3 right   = Vector3::Cross(up,forward).Normalize();
    Vector3 newUp   = Vector3::Cross(forward,right);

    Matrix4x4 lookMat;
    lookMat.m[0][0] = right.x;
    lookMat.m[0][1] = right.y;
    lookMat.m[0][2] = right.z;
    lookMat.m[1][0] = newUp.x;
    lookMat.m[1][1] = newUp.y;
    lookMat.m[1][2] = newUp.z;
    lookMat.m[2][0] = forward.x;
    lookMat.m[2][1] = forward.y;
    lookMat.m[2][2] = forward.z;

    return QuaternionFromMatrix(lookMat);
}

Vector3 Quaternion::GetForwardVector() const {
    // forward = (0,0,1) を回転させたもの
    return TransformNormal({0.0f, 0.0f, 1.0f}, ToMatrix4x4());
}

Matrix4x4 Quaternion::ToMatrix4x4() const {
    Matrix4x4 m{};

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    m.m[0][0] = 1.0f - 2.0f * (yy + zz);
    m.m[0][1] = 2.0f * (xy - wz);
    m.m[0][2] = 2.0f * (xz + wy);
    m.m[0][3] = 0.0f;

    m.m[1][0] = 2.0f * (xy + wz);
    m.m[1][1] = 1.0f - 2.0f * (xx + zz);
    m.m[1][2] = 2.0f * (yz - wx);
    m.m[1][3] = 0.0f;

    m.m[2][0] = 2.0f * (xz - wy);
    m.m[2][1] = 2.0f * (yz + wx);
    m.m[2][2] = 1.0f - 2.0f * (xx + yy);
    m.m[2][3] = 0.0f;

    m.m[3][0] = 0.0f;
    m.m[3][1] = 0.0f;
    m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

Vector3 Quaternion::ToEuler() const {
    Vector3 euler;

    // Pitch (X軸)
    float sinp = 2.0f * (w * x + y * z);
    float cosp = 1.0f - 2.0f * (x * x + y * y);
    euler.x    = std::atan2(sinp, cosp);

    // Yaw (Y軸)
    float siny = 2.0f * (w * y - z * x);
    if (std::abs(siny) >= 1.0f) {
        // ±90度にクランプ
        euler.y = std::copysign(std::numbers::pi_v<float> / 2.0f, siny);
    } else {
        euler.y = std::asin(siny);
    }

    // Roll (Z軸)
    float sinr = 2.0f * (w * z + x * y);
    float cosr = 1.0f - 2.0f * (y * y + z * z);
    euler.z    = std::atan2(sinr, cosr);

    return euler;
}

float Quaternion::Angle(const Quaternion& a, const Quaternion& b) {
    // 2つのクォータニオン間の角度差を計算
    float dot = Quaternion::Dot(a.Normalize(), b.Normalize());

    // 数値誤差を防ぐためにクランプ
    dot = std::clamp(dot, -1.0f, 1.0f);

    // 最短経路の角度を計算
    // 内積が負の場合は補角を使用
    dot = std::abs(dot);

    // アークコサインで角度を求める
    // 2倍するのは、クォータニオンが半角を表すため
    return 2.0f * std::acos(dot);
}
Quaternion Quaternion::FromMatrix(const Matrix4x4& m) {
    Quaternion q{};
    float trace = m.m[0][0] + m.m[1][1] + m.m[2][2]; // 対角成分の合計

    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f) * 2.0f; // s = 4 * qw
        q.w     = 0.25f * s;
        q.x     = (m.m[2][1] - m.m[1][2]) / s;
        q.y     = (m.m[0][2] - m.m[2][0]) / s;
        q.z     = (m.m[1][0] - m.m[0][1]) / s;
    } else if ((m.m[0][0] > m.m[1][1]) && (m.m[0][0] > m.m[2][2])) {
        float s = std::sqrt(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]) * 2.0f; // s = 4 * qx
        q.w     = (m.m[2][1] - m.m[1][2]) / s;
        q.x     = 0.25f * s;
        q.y     = (m.m[0][1] + m.m[1][0]) / s;
        q.z     = (m.m[0][2] + m.m[2][0]) / s;
    } else if (m.m[1][1] > m.m[2][2]) {
        float s = std::sqrt(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]) * 2.0f; // s = 4 * qy
        q.w     = (m.m[0][2] - m.m[2][0]) / s;
        q.x     = (m.m[0][1] + m.m[1][0]) / s;
        q.y     = 0.25f * s;
        q.z     = (m.m[1][2] + m.m[2][1]) / s;
    } else {
        float s = std::sqrt(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]) * 2.0f; // s = 4 * qz
        q.w     = (m.m[1][0] - m.m[0][1]) / s;
        q.x     = (m.m[0][2] + m.m[2][0]) / s;
        q.y     = (m.m[1][2] + m.m[2][1]) / s;
        q.z     = 0.25f * s;
    }

    // 正規化
    float len = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (len > 0.0001f) {
        q.x /= len;
        q.y /= len;
        q.z /= len;
        q.w /= len;
    } else {
        q = Quaternion::Identity();
    }

    return q;
}