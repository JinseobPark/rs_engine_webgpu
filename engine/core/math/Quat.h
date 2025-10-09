#pragma once

#include "Vec3.h"
#include "Mat4.h"
#include <cmath>

namespace rs_engine {

/**
 * @brief Quaternion class for rotation representation
 * 
 * Quaternion (q = w + xi + yj + zk) provides:
 * - Gimbal lock-free rotation
 * - Smooth interpolation (SLERP)
 * - Efficient composition
 * - No singularities at poles
 */
class Quat {
public:
    float w, x, y, z;

    // ========== Constructors ==========
    
    Quat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
    
    Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
    
    // ========== Factory Methods ==========
    
    /**
     * @brief Create identity quaternion (no rotation)
     */
    static Quat identity() {
        return Quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    
    /**
     * @brief Create quaternion from axis and angle
     * @param axis Rotation axis (should be normalized)
     * @param angle Rotation angle in radians
     */
    static Quat fromAxisAngle(const Vec3& axis, float angle) {
        float halfAngle = angle * 0.5f;
        float s = std::sin(halfAngle);
        return Quat(std::cos(halfAngle), axis.x * s, axis.y * s, axis.z * s);
    }
    
    /**
     * @brief Create quaternion from Euler angles (in radians)
     * @param pitch Rotation around X axis (radians)
     * @param yaw Rotation around Y axis (radians)
     * @param roll Rotation around Z axis (radians)
     */
    static Quat fromEuler(float pitch, float yaw, float roll) {
        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);
        float cr = std::cos(roll * 0.5f);
        float sr = std::sin(roll * 0.5f);

        Quat q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;
        return q;
    }
    
    /**
     * @brief Create quaternion looking from eye to target
     *
     * Uses left-handed coordinate system (WebGPU convention):
     * - Forward: +Z axis (into the screen)
     * - Right: +X axis
     * - Up: +Y axis
     *
     * @param forward Forward direction (should be normalized, +Z is forward)
     * @param up Up direction (should be normalized, typically +Y)
     */
    static Quat lookRotation(const Vec3& forward, const Vec3& up) {
        Vec3 f = forward.normalize();
        Vec3 r = f.cross(up).normalize();
        Vec3 u = r.cross(f);
        
        // Build rotation matrix
        float m00 = r.x, m01 = r.y, m02 = r.z;
        float m10 = u.x, m11 = u.y, m12 = u.z;
        float m20 = f.x, m21 = f.y, m22 = f.z;
        
        // Convert to quaternion
        float trace = m00 + m11 + m22;
        Quat q;
        
        if (trace > 0.0f) {
            float s = std::sqrt(trace + 1.0f) * 2.0f;
            q.w = 0.25f * s;
            q.x = (m21 - m12) / s;
            q.y = (m02 - m20) / s;
            q.z = (m10 - m01) / s;
        } else if ((m00 > m11) && (m00 > m22)) {
            float s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
            q.w = (m21 - m12) / s;
            q.x = 0.25f * s;
            q.y = (m01 + m10) / s;
            q.z = (m02 + m20) / s;
        } else if (m11 > m22) {
            float s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
            q.w = (m02 - m20) / s;
            q.x = (m01 + m10) / s;
            q.y = 0.25f * s;
            q.z = (m12 + m21) / s;
        } else {
            float s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
            q.w = (m10 - m01) / s;
            q.x = (m02 + m20) / s;
            q.y = (m12 + m21) / s;
            q.z = 0.25f * s;
        }
        
        return q.normalize();
    }
    
    // ========== Operations ==========
    
    /**
     * @brief Get quaternion magnitude
     */
    float length() const {
        return std::sqrt(w * w + x * x + y * y + z * z);
    }
    
    /**
     * @brief Normalize quaternion
     */
    Quat normalize() const {
        float len = length();
        if (len < 0.0001f) return identity();
        float invLen = 1.0f / len;
        return Quat(w * invLen, x * invLen, y * invLen, z * invLen);
    }
    
    /**
     * @brief Get conjugate (inverse rotation)
     */
    Quat conjugate() const {
        return Quat(w, -x, -y, -z);
    }
    
    /**
     * @brief Get inverse quaternion
     */
    Quat inverse() const {
        float lenSq = w * w + x * x + y * y + z * z;
        if (lenSq < 0.0001f) return identity();
        float invLenSq = 1.0f / lenSq;
        return Quat(w * invLenSq, -x * invLenSq, -y * invLenSq, -z * invLenSq);
    }
    
    /**
     * @brief Quaternion multiplication (rotation composition)
     */
    Quat operator*(const Quat& q) const {
        return Quat(
            w * q.w - x * q.x - y * q.y - z * q.z,
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w
        );
    }
    
    /**
     * @brief Rotate a vector by this quaternion
     */
    Vec3 rotate(const Vec3& v) const {
        Quat q = normalize();
        // q * v * q^-1
        Quat p(0.0f, v.x, v.y, v.z);
        Quat result = q * p * q.conjugate();
        return Vec3(result.x, result.y, result.z);
    }
    
    /**
     * @brief Convert quaternion to rotation matrix
     */
    Mat4 toMatrix() const {
        float xx = x * x, yy = y * y, zz = z * z;
        float xy = x * y, xz = x * z, yz = y * z;
        float wx = w * x, wy = w * y, wz = w * z;
        
        Mat4 m;
        m.m[0] = 1.0f - 2.0f * (yy + zz);
        m.m[1] = 2.0f * (xy + wz);
        m.m[2] = 2.0f * (xz - wy);
        m.m[3] = 0.0f;
        
        m.m[4] = 2.0f * (xy - wz);
        m.m[5] = 1.0f - 2.0f * (xx + zz);
        m.m[6] = 2.0f * (yz + wx);
        m.m[7] = 0.0f;
        
        m.m[8] = 2.0f * (xz + wy);
        m.m[9] = 2.0f * (yz - wx);
        m.m[10] = 1.0f - 2.0f * (xx + yy);
        m.m[11] = 0.0f;
        
        m.m[12] = 0.0f;
        m.m[13] = 0.0f;
        m.m[14] = 0.0f;
        m.m[15] = 1.0f;
        
        return m;
    }
    
    /**
     * @brief Get forward direction vector (+Z axis after rotation)
     *
     * In WebGPU left-handed coordinates, +Z points forward (into screen)
     * This matches the convention used by lookAt and perspective projection
     */
    Vec3 getForward() const {
        return Vec3(
            2.0f * (x * z + w * y),
            2.0f * (y * z - w * x),
            1.0f - 2.0f * (x * x + y * y)
        );
    }
    
    /**
     * @brief Get right direction vector (X axis after rotation)
     */
    Vec3 getRight() const {
        return Vec3(
            1.0f - 2.0f * (y * y + z * z),
            2.0f * (x * y + w * z),
            2.0f * (x * z - w * y)
        );
    }
    
    /**
     * @brief Get up direction vector (Y axis after rotation)
     */
    Vec3 getUp() const {
        return Vec3(
            2.0f * (x * y - w * z),
            1.0f - 2.0f * (x * x + z * z),
            2.0f * (y * z + w * x)
        );
    }
    
    /**
     * @brief Spherical linear interpolation
     * @param q Target quaternion
     * @param t Interpolation factor [0, 1]
     */
    Quat slerp(const Quat& q, float t) const {
        Quat q1 = normalize();
        Quat q2 = q.normalize();
        
        float dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
        
        // If negative, negate one quaternion to take shorter path
        if (dot < 0.0f) {
            q2 = Quat(-q2.w, -q2.x, -q2.y, -q2.z);
            dot = -dot;
        }
        
        // If very close, use linear interpolation
        if (dot > 0.9995f) {
            return Quat(
                q1.w + t * (q2.w - q1.w),
                q1.x + t * (q2.x - q1.x),
                q1.y + t * (q2.y - q1.y),
                q1.z + t * (q2.z - q1.z)
            ).normalize();
        }
        
        float theta = std::acos(dot);
        float sinTheta = std::sin(theta);
        float a = std::sin((1.0f - t) * theta) / sinTheta;
        float b = std::sin(t * theta) / sinTheta;
        
        return Quat(
            q1.w * a + q2.w * b,
            q1.x * a + q2.x * b,
            q1.y * a + q2.y * b,
            q1.z * a + q2.z * b
        );
    }
};

} // namespace rs_engine
