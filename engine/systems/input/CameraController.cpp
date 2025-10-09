#include "CameraController.h"
#include "InputSystem.h"
#include "../../rendering/scene/Camera.h"

namespace rs_engine {

void CameraController::init(InputSystem* inputSys, rendering::Camera* cam) {
    inputSystem = inputSys;
    camera = cam;
    currentMode = Mode::RSEngine;
    
    // Initialize FPS orientation from current camera
    if (camera) {
        Vec3 forward = (camera->getTarget() - camera->getPosition()).normalize();
        Vec3 up = camera->getUp();
        firstPersonOrientation = Quat::lookRotation(forward, up);
        
        // Initialize focal length (RSCamera style)
        focalLength = (camera->getTarget() - camera->getPosition()).length();
        
        // Save initial state in Camera for reset
        camera->saveInitialState();
    } else {
        firstPersonOrientation = Quat::identity();
        focalLength = 10.0f;
    }
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
            Vec3 forward = (camera->getTarget() - camera->getPosition()).normalize();
            Vec3 up = camera->getUp();
            firstPersonOrientation = Quat::lookRotation(forward, up);
        }
    }
}

void CameraController::setTarget(const Vec3& newTarget) {
    if (!camera) return;
    camera->setTarget(newTarget);
}

Vec3 CameraController::getTarget() const {
    return camera ? camera->getTarget() : Vec3(0.0f, 0.0f, 0.0f);
}

float CameraController::getDistance() const {
    if (!camera) return 10.0f;
    return (camera->getPosition() - camera->getTarget()).length();
}

void CameraController::setDistance(float dist) {
    if (!camera) return;
    
    float clampedDist = std::max(minDistance, std::min(maxDistance, dist));
    Vec3 direction = (camera->getPosition() - camera->getTarget()).normalize();
    Vec3 newPosition = camera->getTarget() + direction * clampedDist;
    camera->setPosition(newPosition);
}

void CameraController::reset() {
    if (!camera) return;
    
    // Reset camera to initial state (uses Camera's reset function)
    camera->reset();
    
    // Reset FPS orientation
    Vec3 forward = (camera->getTarget() - camera->getPosition()).normalize();
    Vec3 up = camera->getUp();
    firstPersonOrientation = Quat::lookRotation(forward, up);
    
    // Reset focal length
    focalLength = (camera->getTarget() - camera->getPosition()).length();
    
    // Update camera vectors
    updateCameraVectors();
}

void CameraController::updateRSEngine(float deltaTime) {
    // RSEngine Mode: Maya-style camera control (like old RSCamera)
    // - Combined quaternion rotation (up + right)
    // - Camera and target rotate together around target pivot
    // - Intuitive for 3D content creation
    
    Vec3 cameraPos = camera->getPosition();
    Vec3 target = camera->getTarget();
    
    // Right mouse button: Pan (RSCamera::Move style)
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // RSCamera::Move implementation
        float xoffset = static_cast<float>(dx) * focalLength;
        float yoffset = static_cast<float>(dy) * focalLength;
        
        Vec3 direction = (target - cameraPos).normalize();
        Vec3 viewUp = camera->getUp();
        Vec3 right = direction.cross(viewUp).normalize();
        
        Vec3 rightMovement = right * (xoffset * panSpeed * 0.001f);
        Vec3 upMovement = viewUp * (yoffset * panSpeed * 0.001f);
        
        // Move camera and target together
        camera->setPosition(cameraPos - rightMovement + upMovement);
        camera->setTarget(target - rightMovement + upMovement);
        
        updateCameraVectors();
    }
    
    // Middle mouse button: RSCamera::Rotate style
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // RSCamera::Rotate implementation
        cameraPos = camera->getPosition();
        target = camera->getTarget();
        Vec3 direction = (target - cameraPos).normalize();
        Vec3 viewUp = camera->getUp();
        Vec3 right = direction.cross(viewUp).normalize();
        
        // Create rotation quaternions (matches RSCamera exactly)
        float rotationSensitivity = rotationSpeed * 5.0f;  // Scale to match RSCamera sensitivity
        Quat rotationAroundUp = Quat::fromAxisAngle(
            viewUp, 
            static_cast<float>(dx) * rotationSensitivity * 0.01f
        );
        Quat rotationAroundRight = Quat::fromAxisAngle(
            right, 
            static_cast<float>(dy) * rotationSensitivity * 0.01f
        );
        
        // Combined rotation: note the order (matches original code)
        Quat combinedRotation = rotationAroundUp * rotationAroundRight;
        
        // Rotate camera position around rotation axis
        Vec3 relativePos = cameraPos - rotateAxis;
        Vec3 newPosition = rotateAxis + combinedRotation.rotate(relativePos);
        camera->setPosition(newPosition);
        
        // Rotate focal point around rotation axis
        Vec3 relativeFocus = target - rotateAxis;
        Vec3 newFocalPoint = rotateAxis + combinedRotation.rotate(relativeFocus);
        camera->setTarget(newFocalPoint);
        
        // RSCamera style: UpdateCameraVectors will orthogonalize up vector
        updateCameraVectors();
    }
    
    // Mouse wheel: Zoom (RSCamera::Zoom style)
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        // RSCamera::Zoom implementation
        float sign = scrollDelta * 0.25f;
        cameraPos = camera->getPosition();
        target = camera->getTarget();
        Vec3 direction = (target - cameraPos).normalize();
        float distance = (target - cameraPos).length();
        
        focalLength = distance - sign * distance;
        focalLength = std::max(0.001f, focalLength);
        
        Vec3 newPosition = target - direction * focalLength;
        camera->setPosition(newPosition);
        
        updateCameraVectors();
    }
}

void CameraController::updateTrackball(float deltaTime) {
    // Trackball Mode: RSEngine-style combined rotation (similar to Maya)
    // - Combined quaternion rotation (up + right)
    // - Camera and target rotate together around target pivot
    // - Intuitive for 3D content creation
    
    Vec3 cameraPos = camera->getPosition();
    Vec3 target = camera->getTarget();
    
    // Right mouse button: Pan (RSCamera::Move style)
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // RSCamera::Move implementation
        float xoffset = static_cast<float>(dx) * focalLength;
        float yoffset = static_cast<float>(dy) * focalLength;
        
        Vec3 direction = (target - cameraPos).normalize();
        Vec3 viewUp = camera->getUp();
        Vec3 right = direction.cross(viewUp).normalize();
        
        Vec3 rightMovement = right * (xoffset * panSpeed * 0.001f);
        Vec3 upMovement = viewUp * (yoffset * panSpeed * 0.001f);
        
        // Move camera and target together
        camera->setPosition(cameraPos - rightMovement + upMovement);
        camera->setTarget(target - rightMovement + upMovement);
        
        updateCameraVectors();
    }
    
    // Middle mouse button: RSCamera::Rotate style (combined rotation)
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // RSCamera::Rotate implementation
        cameraPos = camera->getPosition();
        target = camera->getTarget();
        Vec3 direction = (target - cameraPos).normalize();
        Vec3 viewUp = camera->getUp();
        Vec3 right = direction.cross(viewUp).normalize();
        
        // Create rotation quaternions (matches RSCamera exactly)
        float rotationSensitivity = rotationSpeed * 5.0f;  // Scale to match RSCamera sensitivity
        Quat rotationAroundUp = Quat::fromAxisAngle(
            viewUp, 
            static_cast<float>(dx) * rotationSensitivity * 0.01f
        );
        Quat rotationAroundRight = Quat::fromAxisAngle(
            right, 
            static_cast<float>(dy) * rotationSensitivity * 0.01f
        );
        
        // Combined rotation: note the order (matches original code)
        Quat combinedRotation = rotationAroundUp * rotationAroundRight;
        
        // Rotate camera position around rotation axis
        Vec3 relativePos = cameraPos - rotateAxis;
        Vec3 newPosition = rotateAxis + combinedRotation.rotate(relativePos);
        camera->setPosition(newPosition);
        
        // Rotate focal point around rotation axis
        Vec3 relativeFocus = target - rotateAxis;
        Vec3 newFocalPoint = rotateAxis + combinedRotation.rotate(relativeFocus);
        camera->setTarget(newFocalPoint);
        
        // RSCamera style: UpdateCameraVectors will orthogonalize up vector
        updateCameraVectors();
    }
    
    // Mouse wheel: Zoom (RSCamera::Zoom style)
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        // RSCamera::Zoom implementation
        float sign = scrollDelta * 0.25f;
        cameraPos = camera->getPosition();
        target = camera->getTarget();
        Vec3 direction = (target - cameraPos).normalize();
        float distance = (target - cameraPos).length();
        
        focalLength = distance - sign * distance;
        focalLength = std::max(0.001f, focalLength);
        
        Vec3 newPosition = target - direction * focalLength;
        camera->setPosition(newPosition);
        
        updateCameraVectors();
    }
}

void CameraController::updateOrbit(float deltaTime) {
    // Orbit Mode: RSEngine-style combined rotation (constrained to Y axis)
    // - Combined quaternion rotation (world Y + camera right)
    // - Camera and target rotate together around target pivot
    // - Constrained horizontal rotation for stable orbit
    
    Vec3 cameraPos = camera->getPosition();
    Vec3 target = camera->getTarget();
    
    // Right mouse button: Pan (RSCamera::Move style)
    if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // RSCamera::Move implementation
        float xoffset = static_cast<float>(dx) * focalLength;
        float yoffset = static_cast<float>(dy) * focalLength;
        
        Vec3 direction = (target - cameraPos).normalize();
        Vec3 viewUp = camera->getUp();
        Vec3 right = direction.cross(viewUp).normalize();
        
        Vec3 rightMovement = right * (xoffset * panSpeed * 0.001f);
        Vec3 upMovement = viewUp * (yoffset * panSpeed * 0.001f);
        
        // Move camera and target together
        camera->setPosition(cameraPos - rightMovement + upMovement);
        camera->setTarget(target - rightMovement + upMovement);
        
        updateCameraVectors();
    }
    
    // Middle mouse button: Rotation (Orbit style - constrained to world Y axis)
    if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
        double dx, dy;
        inputSystem->getMouseDelta(dx, dy);
        
        // RSCamera::Rotate implementation with Y-axis constraint
        cameraPos = camera->getPosition();
        target = camera->getTarget();
        Vec3 direction = (target - cameraPos).normalize();
        Vec3 viewUp = camera->getUp();
        Vec3 right = direction.cross(viewUp).normalize();
        
        // Create rotation quaternions
        // Horizontal: constrained to world Y axis
        // Vertical: use camera's right axis
        float rotationSensitivity = rotationSpeed * 5.0f;  // Scale to match RSCamera sensitivity
        Quat rotationAroundWorldY = Quat::fromAxisAngle(
            Vec3(0.0f, 1.0f, 0.0f),  // World Y axis constraint
            static_cast<float>(dx) * rotationSensitivity * 0.01f
        );
        Quat rotationAroundRight = Quat::fromAxisAngle(
            right, 
            static_cast<float>(dy) * rotationSensitivity * 0.01f
        );
        
        // Combined rotation: note the order (matches original code)
        Quat combinedRotation = rotationAroundWorldY * rotationAroundRight;
        
        // Rotate camera position around rotation axis
        Vec3 relativePos = cameraPos - rotateAxis;
        Vec3 newPosition = rotateAxis + combinedRotation.rotate(relativePos);
        camera->setPosition(newPosition);
        
        // Rotate focal point around rotation axis
        Vec3 relativeFocus = target - rotateAxis;
        Vec3 newFocalPoint = rotateAxis + combinedRotation.rotate(relativeFocus);
        camera->setTarget(newFocalPoint);
        
        // RSCamera style: UpdateCameraVectors will orthogonalize up vector
        updateCameraVectors();
    }
    
    // Mouse wheel: Zoom (RSCamera::Zoom style)
    float scrollDelta = inputSystem->getMouseScrollDelta();
    if (std::abs(scrollDelta) > 0.001f) {
        // RSCamera::Zoom implementation
        float sign = scrollDelta * 0.25f;
        cameraPos = camera->getPosition();
        target = camera->getTarget();
        Vec3 direction = (target - cameraPos).normalize();
        float distance = (target - cameraPos).length();
        
        focalLength = distance - sign * distance;
        focalLength = std::max(0.001f, focalLength);
        
        Vec3 newPosition = target - direction * focalLength;
        camera->setPosition(newPosition);
        
        updateCameraVectors();
    }
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
    
    Vec3 newTarget = newPosition + forward;
    camera->setTarget(newTarget);
    // Use quaternion's up vector to avoid gimbal lock when looking straight up/down
    Vec3 up = firstPersonOrientation.getUp();
    camera->lookAt(newPosition, newTarget, up);
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
    
    Vec3 newTarget = newPosition + forward;
    camera->setTarget(newTarget);
    // Use quaternion's up vector for true 6DOF movement (no gimbal lock!)
    camera->lookAt(newPosition, newTarget, up);
}

Quat CameraController::getCurrentOrientation() const {
    if (!camera) return Quat::identity();
    
    Vec3 forward = (camera->getTarget() - camera->getPosition()).normalize();
    Vec3 up = camera->getUp();
    
    return Quat::lookRotation(forward, up);
}

void CameraController::updateCameraVectors() {
    if (!camera) return;
    
    // RSCamera::UpdateCameraVectors implementation
    // Orthogonalize up vector to be perpendicular to view direction
    Vec3 target = camera->getTarget();
    Vec3 position = camera->getPosition();
    Vec3 direction = (target - position).normalize();
    Vec3 viewUp = camera->getUp();
    
    // Gram-Schmidt orthogonalization:
    // 1. Get right vector: right = direction × up
    // 2. Get orthogonal up: up = (direction × up) × direction
    // This ensures up is perpendicular to direction
    Vec3 right = direction.cross(viewUp).normalize();
    Vec3 orthogonalUp = right.cross(direction).normalize();
    
    camera->setUp(orthogonalUp);
    camera->lookAt(position, target, orthogonalUp);
    
    // Update focal length
    focalLength = (target - position).length();
}

} // namespace rs_engine
