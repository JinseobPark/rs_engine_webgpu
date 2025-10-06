#include "CameraController.h"
#include "InputSystem.h"
#include "../../rendering/scene/Camera.h"

namespace rs_engine {

void CameraController::init(InputSystem* inputSys, rendering::Camera* cam) {
    inputSystem = inputSys;
    camera = cam;
    currentMode = Mode::RSEngine;
    
    // Default target and distance
    target = Vec3(0.0f, 0.0f, 0.0f);
    distance = 10.0f;
    
    // Initialize orientation from current camera position using quaternions
    if (camera) {
        Vec3 currentPos = camera->getPosition();
        Vec3 toTarget = target - currentPos;
        distance = toTarget.length();
        
        if (distance > 0.001f) {
            // Calculate initial orientation looking at target
            Vec3 forward = toTarget.normalize();
            Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
            orientation = Quat::lookRotation(forward, up);
            firstPersonOrientation = orientation;
        } else {
            orientation = Quat::identity();
            firstPersonOrientation = Quat::identity();
        }
    }
    
    // Save initial state for reset
    initialPosition = camera ? camera->getPosition() : Vec3(0.0f, 5.0f, 10.0f);
    initialTarget = target;
    initialDistance = distance;
    initialOrientation = orientation;
}

void CameraController::update(float deltaTime) {
    if (!camera || !inputSystem) return;
    
    switch (currentMode) {
        case Mode::RSEngine:
            updateRSEngine(deltaTime);
            break;
        case Mode::Trackball:
            updateTrackball(deltaTime);
            break;
        case Mode::Orbit:
            updateOrbit(deltaTime);
            break;
        case Mode::FirstPerson:
            updateFirstPerson(deltaTime);
            break;
        case Mode::Free:
            updateFree(deltaTime);
            break;
    }
}

void CameraController::setMode(Mode mode) {
    currentMode = mode;
    
    if (mode == Mode::FirstPerson || mode == Mode::Free) {
        // Initialize FPS orientation from current camera direction
        if (camera) {
            Vec3 forward = (target - camera->getPosition()).normalize();
            Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
            firstPersonOrientation = Quat::lookRotation(forward, up);
        }
    }
}

void CameraController::setTarget(const Vec3& newTarget) {
    target = newTarget;
}

void CameraController::setDistance(float dist) {
    distance = std::max(minDistance, std::min(maxDistance, dist));
}

void CameraController::reset() {
    // Restore initial state
    target = initialTarget;
    distance = initialDistance;
    orientation = initialOrientation;
    firstPersonOrientation = initialOrientation;
    
    // Update camera position
    if (camera) {
        camera->setPosition(initialPosition);
        camera->lookAt(initialPosition, target, Vec3(0.0f, 1.0f, 0.0f));
    }
}

void CameraController::updateRSEngine(float deltaTime) {
    // RSEngine Mode: Maya-style camera control (like old RSCamera)
    // - Combined quaternion rotation (up + right)
    // - Camera and target rotate together around target pivot
    // - Intuitive for 3D content creation
    
    // Right mouse button: Pan (move camera and target together)
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        Vec3 right = orientation.getRight();
        Vec3 up = orientation.getUp();
        
        Vec3 panOffset = right * (static_cast<float>(dx) * panSpeed * distance * 0.001f) + 
                         up * (-static_cast<float>(dy) * panSpeed * distance * 0.001f);
        
        // Maya-style: Move camera and target together
        Vec3 cameraPos = camera->getPosition();
        camera->setPosition(cameraPos + panOffset);
        target = target + panOffset;
    }
    
    // Middle mouse button: Maya-style combined rotation
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // Calculate rotation axes from current view
        Vec3 direction = (target - camera->getPosition()).normalize();
        Vec3 viewUp = orientation.getUp();  // Use current up vector
        Vec3 right = direction.cross(viewUp).normalize();
        
        // Maya-style: Combined rotation (up first, then right)
        // This matches the original RSCamera::Rotate implementation
        Quat rotationAroundUp = Quat::fromAxisAngle(
            viewUp, 
            static_cast<float>(dx) * rotationSpeed * 0.01f
        );
        Quat rotationAroundRight = Quat::fromAxisAngle(
            right, 
            static_cast<float>(dy) * rotationSpeed * 0.01f
        );
        
        // Combined rotation: note the order (matches original code)
        Quat combinedRotation = rotationAroundUp * rotationAroundRight;
        
        // Rotate camera around target (rotate_axis_ in original code)
        Vec3 cameraPos = camera->getPosition();
        Vec3 relativePos = cameraPos - target;
        Vec3 newPosition = target + combinedRotation.rotate(relativePos);
        camera->setPosition(newPosition);
        
        // Update orientation
        orientation = combinedRotation * orientation;
        orientation = orientation.normalize();
        
        // Recalculate distance
        distance = (newPosition - target).length();
    }
    
    // Mouse wheel: Zoom (move camera towards/away from target)
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        Vec3 cameraPos = camera->getPosition();
        Vec3 toTarget = (target - cameraPos).normalize();
        Vec3 movement = toTarget * scrollDelta * zoomSpeed * 0.1f;
        
        Vec3 newPosition = cameraPos + movement;
        camera->setPosition(newPosition);
        
        // Update distance and orientation
        distance = (target - newPosition).length();
        Vec3 newDirection = (target - newPosition).normalize();
        Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
        orientation = Quat::lookRotation(newDirection, up);
    }
    
    camera->lookAt(camera->getPosition(), target, Vec3(0.0f, 1.0f, 0.0f));
}

void CameraController::updateTrackball(float deltaTime) {
    // Right mouse button: Pan
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // Get camera's right and up vectors from quaternion
        Vec3 right = orientation.getRight();
        Vec3 up = orientation.getUp();
        
        // Inverted direction for intuitive panning
        target = target + right * (static_cast<float>(dx) * panSpeed * distance * 0.001f) + 
                         up * (-static_cast<float>(dy) * panSpeed * distance * 0.001f);
    }
    
    // Middle mouse button: Rotation (Three.js Trackball style - true virtual trackball)
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // Three.js TrackballControls algorithm:
        // 1. Calculate mouse movement in screen space (right + up)
        // 2. Find rotation axis perpendicular to both mouse movement and eye vector
        // 3. Rotate around that axis
        
        Vec3 eye = camera->getPosition() - target;  // Camera to target vector
        Vec3 eyeDirection = eye.normalize();
        
        // Screen space basis vectors (from current orientation)
        Vec3 objectUp = orientation.getUp().normalize();
        Vec3 objectSideways = objectUp.cross(eyeDirection).normalize();
        
        // Mouse movement in screen space
        Vec3 moveDirection = objectSideways * static_cast<float>(dx) * rotationSpeed * 0.01f +
                            objectUp * static_cast<float>(dy) * rotationSpeed * 0.01f;
        
        float moveLength = moveDirection.length();
        
        if (moveLength > 0.0001f) {
            // Rotation axis = cross product of movement and eye vector
            // This creates the "rolling ball" effect
            Vec3 axis = moveDirection.cross(eye).normalize();
            float angle = moveLength;
            
            // Create rotation quaternion
            Quat rotation = Quat::fromAxisAngle(axis, angle);
            
            // Apply rotation to orientation (this rotates both eye and up)
            orientation = rotation * orientation;
            orientation = orientation.normalize();
        }
    }
    
    // Mouse wheel: Zoom
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        distance -= scrollDelta * zoomSpeed;
        distance = std::max(minDistance, std::min(maxDistance, distance));
    }
    
    // Calculate camera position from quaternion orientation (no trigonometry!)
    Vec3 forward = orientation.getForward();
    Vec3 newPosition = target - forward * distance;
    
    camera->setPosition(newPosition);
    camera->lookAt(newPosition, target, Vec3(0.0f, 1.0f, 0.0f));
}

void CameraController::updateOrbit(float deltaTime) {
    // Right mouse button: Pan
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // Get camera's right vector, use world up
        Vec3 right = orientation.getRight();
        Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
        
        // Inverted direction for intuitive panning
        target = target + right * (static_cast<float>(dx) * panSpeed * distance * 0.001f) + 
                         up * (-static_cast<float>(dy) * panSpeed * distance * 0.001f);
    }
    
    // Middle mouse button: Rotation (Orbit style - constrained to Y axis, NO gimbal lock!)
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // Horizontal rotation around world Y axis (like Trackball)
        Quat yawRotation = Quat::fromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 
                                                static_cast<float>(dx) * rotationSpeed * 0.01f);
        
        // Vertical rotation around camera's local right axis
        Vec3 rightAxis = orientation.getRight();
        Quat pitchRotation = Quat::fromAxisAngle(rightAxis, 
                                                   static_cast<float>(dy) * rotationSpeed * 0.01f);
        
        // NO ANGLE CLAMPING - full 360° freedom!
        orientation = yawRotation * pitchRotation * orientation;
        orientation = orientation.normalize();
    }
    
    // Mouse wheel: Zoom
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        distance -= scrollDelta * zoomSpeed;
        distance = std::max(minDistance, std::min(maxDistance, distance));
    }
    
    // Calculate camera position from quaternion
    Vec3 forward = orientation.getForward();
    Vec3 newPosition = target - forward * distance;
    
    camera->setPosition(newPosition);
    camera->lookAt(newPosition, target, Vec3(0.0f, 1.0f, 0.0f));
}

void CameraController::updateFirstPerson(float deltaTime) {
    // Middle mouse button: Look around (NO gimbal lock - full 360° freedom!)
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // Horizontal rotation around world Y axis
        Quat yawRotation = Quat::fromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 
                                                static_cast<float>(dx) * rotationSpeed * 0.1f);
        
        // Vertical rotation around camera's local right axis
        Vec3 rightAxis = firstPersonOrientation.getRight();
        Quat pitchRotation = Quat::fromAxisAngle(rightAxis, 
                                                   static_cast<float>(dy) * rotationSpeed * 0.1f);
        
        // NO PITCH CLAMPING - you can now look straight up and straight down!
        firstPersonOrientation = yawRotation * pitchRotation * firstPersonOrientation;
        firstPersonOrientation = firstPersonOrientation.normalize();
    }
    
    // Calculate look direction from quaternion (no trigonometry needed!)
    Vec3 forward = firstPersonOrientation.getForward();
    Vec3 right = firstPersonOrientation.getRight();
    
    // WASD movement
    Vec3 movement(0.0f, 0.0f, 0.0f);
    float moveSpeed = firstPersonMoveSpeed * deltaTime;
    
    if (inputSystem->isKeyDown(KeyCode::W)) movement = movement + forward * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::S)) movement = movement - forward * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::A)) movement = movement - right * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::D)) movement = movement + right * moveSpeed;
    
    // Right mouse button: Pan (strafe up/down)
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        movement = movement + Vec3(0.0f, static_cast<float>(dy) * panSpeed * moveSpeed * 0.1f, 0.0f);
    }
    
    // Mouse wheel: Move forward/backward
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        movement = movement + forward * scrollDelta * zoomSpeed * 0.1f;
    }
    
    Vec3 newPosition = camera->getPosition() + movement;
    camera->setPosition(newPosition);
    
    target = newPosition + forward;
    camera->lookAt(newPosition, target, Vec3(0.0f, 1.0f, 0.0f));
}

void CameraController::updateFree(float deltaTime) {
    // Middle mouse button: Look around (fully free, NO constraints, NO gimbal lock!)
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // Horizontal rotation around world Y axis
        Quat yawRotation = Quat::fromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 
                                                static_cast<float>(dx) * rotationSpeed * 0.1f);
        
        // Vertical rotation around camera's local right axis
        Vec3 rightAxis = firstPersonOrientation.getRight();
        Quat pitchRotation = Quat::fromAxisAngle(rightAxis, 
                                                   static_cast<float>(dy) * rotationSpeed * 0.1f);
        
        // TRULY FREE - no limitations whatsoever!
        firstPersonOrientation = yawRotation * pitchRotation * firstPersonOrientation;
        firstPersonOrientation = firstPersonOrientation.normalize();
    }
    
    // Calculate direction vectors from quaternion (perfect precision!)
    Vec3 forward = firstPersonOrientation.getForward();
    Vec3 right = firstPersonOrientation.getRight();
    Vec3 up = firstPersonOrientation.getUp();
    
    // WASDQE movement (6DOF - true free flight!)
    Vec3 movement(0.0f, 0.0f, 0.0f);
    float moveSpeed = firstPersonMoveSpeed * deltaTime;
    
    if (inputSystem->isKeyDown(KeyCode::W)) movement = movement + forward * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::S)) movement = movement - forward * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::A)) movement = movement - right * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::D)) movement = movement + right * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::Q)) movement = movement - up * moveSpeed;
    if (inputSystem->isKeyDown(KeyCode::E)) movement = movement + up * moveSpeed;
    
    // Right mouse button: Pan
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        movement = movement + right * (static_cast<float>(dx) * panSpeed * moveSpeed * 0.1f) + 
                             up * (-static_cast<float>(dy) * panSpeed * moveSpeed * 0.1f);
    }
    
    // Mouse wheel: Move forward/backward
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        movement = movement + forward * scrollDelta * zoomSpeed * 0.1f;
    }
    
    Vec3 newPosition = camera->getPosition() + movement;
    camera->setPosition(newPosition);
    
    target = newPosition + forward;
    camera->lookAt(newPosition, target, Vec3(0.0f, 1.0f, 0.0f));
}

} // namespace rs_engine
