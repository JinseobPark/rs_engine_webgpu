#pragma once

#include "../../core/math/Vec3.h"
#include <array>
#include <cmath>

namespace rs_engine {

// Forward declare Mat4 to avoid circular dependency
struct Mat4 {
    float data[16];

    Mat4() {
        // Identity matrix
        for (int i = 0; i < 16; i++) data[i] = 0.0f;
        data[0] = data[5] = data[10] = data[15] = 1.0f;
    }

    static Mat4 perspective(float fov, float aspect, float near, float far);
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
    static Mat4 multiply(const Mat4& a, const Mat4& b);
    Mat4 transpose() const;
};

namespace rendering {

class Camera {
private:
    Vec3 position;
    Vec3 target;
    Vec3 up;
    
    float fov;      // Field of view in radians
    float aspect;   // Aspect ratio
    float nearPlane;
    float farPlane;
    
    mutable Mat4 viewMatrix;
    mutable Mat4 projectionMatrix;
    mutable Mat4 viewProjMatrix;
    mutable bool viewDirty = true;
    mutable bool projDirty = true;

public:
    Camera(float fov = 45.0f * M_PI / 180.0f, float aspect = 4.0f/3.0f, 
           float near = 0.1f, float far = 100.0f);

    // Position and orientation
    void setPosition(const Vec3& pos);
    void setTarget(const Vec3& target);
    void setUp(const Vec3& up);
    void lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);

    // Projection parameters
    void setPerspective(float fov, float aspect, float near, float far);
    void setAspectRatio(float aspect);

    // Getters
    const Vec3& getPosition() const { return position; }
    const Vec3& getTarget() const { return target; }
    const Vec3& getUp() const { return up; }
    
    const Mat4& getViewMatrix() const;
    const Mat4& getProjectionMatrix() const;
    const Mat4& getViewProjectionMatrix() const;

    // Update camera (call this if you modify position/target directly)
    void update();

private:
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;
};

} // namespace rendering
} // namespace rs_engine