#include "Camera.h"
#include <iostream>

namespace rs_engine {
namespace rendering {

Camera::Camera(float fov, float aspect, float near, float far)
    : position(0, 0, 5), target(0, 0, 0), up(0, 1, 0)
    , initialPosition(0, 0, 5), initialTarget(0, 0, 0), initialUp(0, 1, 0)
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

void Camera::setFOV(float fovDegrees) {
    fov = fovDegrees * M_PI / 180.0f;
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
        viewProjMatrix = projectionMatrix * viewMatrix;
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

void Camera::reset() {
    // Restore initial state
    position = initialPosition;
    target = initialTarget;
    up = initialUp;
    viewDirty = true;
}

void Camera::saveInitialState() {
    // Save current state as initial
    initialPosition = position;
    initialTarget = target;
    initialUp = up;
}

} // namespace rendering
} // namespace rs_engine