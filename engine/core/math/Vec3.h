#pragma once

#include <array>
#include <cmath>

namespace rs_engine {

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(const std::array<float, 3>& arr) : x(arr[0]), y(arr[1]), z(arr[2]) {}

    // Convert to array for compatibility
    std::array<float, 3> toArray() const {
        return {x, y, z};
    }

    // Operators
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    // Dot product
    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Cross product
    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // Length
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Normalize
    Vec3 normalize() const {
        float len = length();
        if (len > 0.0f) {
            return *this / len;
        }
        return Vec3(0, 0, 0);
    }
    
    // Alias for normalize (for consistency)
    Vec3 normalized() const {
        return normalize();
    }
    
    // Array access operator
    float operator[](int index) const {
        return (index == 0) ? x : (index == 1) ? y : z;
    }
    
    float& operator[](int index) {
        return (index == 0) ? x : (index == 1) ? y : z;
    }
};

} // namespace rs_engine