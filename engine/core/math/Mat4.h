#pragma once

#include <array>
#include <cmath>
#include "Vec3.h"

namespace rs_engine {

struct Mat4 {
    // Column-major order (standard for graphics)
    float m[16];

    Mat4() {
        identity();
    }

    Mat4(const std::array<float, 16>& arr) {
        for (int i = 0; i < 16; ++i) {
            m[i] = arr[i];
        }
    }

    Mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33) {
        m[0] = m00; m[1] = m10; m[2] = m20; m[3] = m30;
        m[4] = m01; m[5] = m11; m[6] = m21; m[7] = m31;
        m[8] = m02; m[9] = m12; m[10] = m22; m[11] = m32;
        m[12] = m03; m[13] = m13; m[14] = m23; m[15] = m33;
    }

    // Convert to array for compatibility
    std::array<float, 16> toArray() const {
        std::array<float, 16> arr;
        for (int i = 0; i < 16; ++i) {
            arr[i] = m[i];
        }
        return arr;
    }

    // Element access
    float& operator()(int row, int col) {
        return m[col * 4 + row];
    }

    const float& operator()(int row, int col) const {
        return m[col * 4 + row];
    }

    // Matrix operations
    Mat4 operator+(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 16; ++i) {
            result.m[i] = m[i] + other.m[i];
        }
        return result;
    }

    Mat4 operator-(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 16; ++i) {
            result.m[i] = m[i] - other.m[i];
        }
        return result;
    }

    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                result(row, col) = 0;
                for (int k = 0; k < 4; ++k) {
                    result(row, col) += (*this)(row, k) * other(k, col);
                }
            }
        }
        return result;
    }

    Mat4 operator*(float scalar) const {
        Mat4 result;
        for (int i = 0; i < 16; ++i) {
            result.m[i] = m[i] * scalar;
        }
        return result;
    }

    // Matrix-vector multiplication
    Vec3 operator*(const Vec3& vec) const {
        return Vec3(
            (*this)(0, 0) * vec.x + (*this)(0, 1) * vec.y + (*this)(0, 2) * vec.z + (*this)(0, 3),
            (*this)(1, 0) * vec.x + (*this)(1, 1) * vec.y + (*this)(1, 2) * vec.z + (*this)(1, 3),
            (*this)(2, 0) * vec.x + (*this)(2, 1) * vec.y + (*this)(2, 2) * vec.z + (*this)(2, 3)
        );
    }

    // Identity matrix
    void identity() {
        for (int i = 0; i < 16; ++i) {
            m[i] = 0.0f;
        }
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    // Transpose
    Mat4 transpose() const {
        Mat4 result;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                result(col, row) = (*this)(row, col);
            }
        }
        return result;
    }

    // Determinant
    float determinant() const {
        float det = 0.0f;

        // Calculate determinant using cofactor expansion along first row
        det += (*this)(0, 0) * (
            (*this)(1, 1) * ((*this)(2, 2) * (*this)(3, 3) - (*this)(2, 3) * (*this)(3, 2)) -
            (*this)(1, 2) * ((*this)(2, 1) * (*this)(3, 3) - (*this)(2, 3) * (*this)(3, 1)) +
            (*this)(1, 3) * ((*this)(2, 1) * (*this)(3, 2) - (*this)(2, 2) * (*this)(3, 1))
        );

        det -= (*this)(0, 1) * (
            (*this)(1, 0) * ((*this)(2, 2) * (*this)(3, 3) - (*this)(2, 3) * (*this)(3, 2)) -
            (*this)(1, 2) * ((*this)(2, 0) * (*this)(3, 3) - (*this)(2, 3) * (*this)(3, 0)) +
            (*this)(1, 3) * ((*this)(2, 0) * (*this)(3, 2) - (*this)(2, 2) * (*this)(3, 0))
        );

        det += (*this)(0, 2) * (
            (*this)(1, 0) * ((*this)(2, 1) * (*this)(3, 3) - (*this)(2, 3) * (*this)(3, 1)) -
            (*this)(1, 1) * ((*this)(2, 0) * (*this)(3, 3) - (*this)(2, 3) * (*this)(3, 0)) +
            (*this)(1, 3) * ((*this)(2, 0) * (*this)(3, 1) - (*this)(2, 1) * (*this)(3, 0))
        );

        det -= (*this)(0, 3) * (
            (*this)(1, 0) * ((*this)(2, 1) * (*this)(3, 2) - (*this)(2, 2) * (*this)(3, 1)) -
            (*this)(1, 1) * ((*this)(2, 0) * (*this)(3, 2) - (*this)(2, 2) * (*this)(3, 0)) +
            (*this)(1, 2) * ((*this)(2, 0) * (*this)(3, 1) - (*this)(2, 1) * (*this)(3, 0))
        );

        return det;
    }

    // Static factory methods for common transformations
    static Mat4 translation(const Vec3& translation) {
        Mat4 result;
        result.identity();
        result(0, 3) = translation.x;
        result(1, 3) = translation.y;
        result(2, 3) = translation.z;
        return result;
    }

    static Mat4 scale(const Vec3& scale) {
        Mat4 result;
        result.identity();
        result(0, 0) = scale.x;
        result(1, 1) = scale.y;
        result(2, 2) = scale.z;
        return result;
    }

    static Mat4 rotationX(float angle) {
        Mat4 result;
        result.identity();
        float c = std::cos(angle);
        float s = std::sin(angle);
        result(1, 1) = c;
        result(1, 2) = -s;
        result(2, 1) = s;
        result(2, 2) = c;
        return result;
    }

    static Mat4 rotationY(float angle) {
        Mat4 result;
        result.identity();
        float c = std::cos(angle);
        float s = std::sin(angle);
        result(0, 0) = c;
        result(0, 2) = s;
        result(2, 0) = -s;
        result(2, 2) = c;
        return result;
    }

    static Mat4 rotationZ(float angle) {
        Mat4 result;
        result.identity();
        float c = std::cos(angle);
        float s = std::sin(angle);
        result(0, 0) = c;
        result(0, 1) = -s;
        result(1, 0) = s;
        result(1, 1) = c;
        return result;
    }

    static Mat4 perspective(float fov, float aspect, float near, float far) {
        Mat4 result;
        float tanHalfFov = std::tan(fov * 0.5f);

        result.m[0] = 1.0f / (aspect * tanHalfFov);
        result.m[5] = 1.0f / tanHalfFov;
        result.m[10] = -(far + near) / (far - near);
        result.m[11] = -1.0f;
        result.m[14] = -(2.0f * far * near) / (far - near);
        result.m[15] = 0.0f;

        for (int i = 1; i < 5; ++i) if (i != 1) result.m[i] = 0.0f;
        for (int i = 6; i < 10; ++i) if (i != 6) result.m[i] = 0.0f;
        result.m[12] = result.m[13] = 0.0f;

        return result;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalize();
        Vec3 s = f.cross(up).normalize();
        Vec3 u = s.cross(f);

        Mat4 result;
        result.identity();

        result(0, 0) = s.x;
        result(1, 0) = s.y;
        result(2, 0) = s.z;
        result(0, 1) = u.x;
        result(1, 1) = u.y;
        result(2, 1) = u.z;
        result(0, 2) = -f.x;
        result(1, 2) = -f.y;
        result(2, 2) = -f.z;
        result(0, 3) = -s.dot(eye);
        result(1, 3) = -u.dot(eye);
        result(2, 3) = f.dot(eye);

        return result;
    }
};

} // namespace rs_engine