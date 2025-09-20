#include "Camera.h"
#include <iostream>

namespace rs_engine {

// Mat4 implementation (moved from CubeRenderer)
Mat4 Mat4::perspective(float fov, float aspect, float near, float far) {
    Mat4 result = Mat4(); // Initialize to zero
    float tanHalfFov = tan(fov / 2.0f);

    result.data[0] = 1.0f / (aspect * tanHalfFov);
    result.data[5] = 1.0f / tanHalfFov;
    result.data[10] = -(far + near) / (far - near);
    result.data[11] = -1.0f;
    result.data[14] = -(2.0f * far * near) / (far - near);
    result.data[15] = 0.0f;

    return result;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Mat4 result = Mat4(); // Initialize to zero

    // Calculate forward vector (from eye to center)
    Vec3 forward = (center - eye).normalize();
    
    // Calculate right vector (cross product of forward and up)
    Vec3 right = forward.cross(up).normalize();
    
    // Calculate actual up vector (cross product of right and forward)
    Vec3 actualUp = right.cross(forward);

    // Build view matrix (note: we want right-handed coordinate system)
    result.data[0] = right.x; result.data[1] = actualUp.x; result.data[2] = -forward.x; result.data[3] = 0;
    result.data[4] = right.y; result.data[5] = actualUp.y; result.data[6] = -forward.y; result.data[7] = 0;
    result.data[8] = right.z; result.data[9] = actualUp.z; result.data[10] = -forward.z; result.data[11] = 0;
    result.data[12] = -(right.dot(eye));
    result.data[13] = -(actualUp.dot(eye));
    result.data[14] = -(-forward.dot(eye));
    result.data[15] = 1;

    return result;
}

Mat4 Mat4::multiply(const Mat4& a, const Mat4& b) {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.data[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result.data[i * 4 + j] += a.data[i * 4 + k] * b.data[k * 4 + j];
            }
        }
    }
    return result;
}

Mat4 Mat4::transpose() const {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.data[i * 4 + j] = data[j * 4 + i];
        }
    }
    return result;
}

namespace rendering {

Camera::Camera(float fov, float aspect, float near, float far)
    : position(0, 0, 5), target(0, 0, 0), up(0, 1, 0)
    , fov(fov), aspect(aspect), nearPlane(near), farPlane(far)
    , viewDirty(true), projDirty(true) {
}

void Camera::setPosition(const Vec3& pos) {
    position = pos;
    viewDirty = true;
}

void Camera::setTarget(const Vec3& tgt) {
    target = tgt;
    viewDirty = true;
}

void Camera::setUp(const Vec3& upVec) {
    up = upVec;
    viewDirty = true;
}

void Camera::lookAt(const Vec3& eye, const Vec3& center, const Vec3& upVec) {
    position = eye;
    target = center;
    up = upVec;
    viewDirty = true;
}

void Camera::setPerspective(float fov, float aspect, float near, float far) {
    this->fov = fov;
    this->aspect = aspect;
    this->nearPlane = near;
    this->farPlane = far;
    projDirty = true;
}

void Camera::setAspectRatio(float aspect) {
    this->aspect = aspect;
    projDirty = true;
}

const Mat4& Camera::getViewMatrix() const {
    if (viewDirty) {
        updateViewMatrix();
    }
    return viewMatrix;
}

const Mat4& Camera::getProjectionMatrix() const {
    if (projDirty) {
        updateProjectionMatrix();
    }
    return projectionMatrix;
}

const Mat4& Camera::getViewProjectionMatrix() const {
    if (viewDirty || projDirty) {
        if (viewDirty) updateViewMatrix();
        if (projDirty) updateProjectionMatrix();
        viewProjMatrix = Mat4::multiply(viewMatrix, projectionMatrix);
    }
    return viewProjMatrix;
}

void Camera::update() {
    viewDirty = true;
    projDirty = true;
}

void Camera::updateViewMatrix() const {
    viewMatrix = Mat4::lookAt(position, target, up);
    viewDirty = false;
}

void Camera::updateProjectionMatrix() const {
    projectionMatrix = Mat4::perspective(fov, aspect, nearPlane, farPlane);
    projDirty = false;
}

} // namespace rendering
} // namespace rs_engine