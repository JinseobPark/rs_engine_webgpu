#pragma once

#include "InputSystem.h"
#include "../../rendering/scene/Camera.h"
#include "../../core/math/Vec3.h"
#include <cmath>

namespace rs_engine {

/**
 * @brief Multi-Mode Camera Controller using InputSystem
 * 
 * Modes:
 * - Trackball: Spherical rotation around target (default)
 * - Orbit: Cylindrical rotation around target
 * - FirstPerson: Classic FPS-style movement
 * - Free: Free-flying camera
 * 
 * Common Controls:
 * - Left Mouse: (Reserved for object selection - not used by camera)
 * - Right Mouse Drag: Pan movement
 * - Mouse Wheel: Zoom in/out
 * - Mouse Wheel Drag (Middle Button): Rotation
 */
class CameraController {
public:
    enum class Mode {
        Trackball,    // Default: Spherical rotation (like 3D modeling tools)
        Orbit,        // Cylindrical rotation (horizontal + vertical)
        FirstPerson,  // FPS-style movement
        Free          // Free-flying camera
    };

private:
    InputSystem* inputSystem = nullptr;
    rendering::Camera* camera = nullptr;
    
    // Current mode
    Mode currentMode = Mode::Trackball;
    
    // Camera state
    Vec3 position;
    float yaw = 0.0f;      // Horizontal rotation
    float pitch = 0.0f;    // Vertical rotation
    
    // Trackball/Orbit mode parameters
    Vec3 targetPoint = Vec3(0.0f, 0.0f, 0.0f);  // Look-at point
    float distance = 5.0f;                       // Distance from target
    
    // Movement parameters
    float moveSpeed = 5.0f;
    float panSpeed = 0.01f;
    float zoomSpeed = 1.0f;
    float rotationSensitivity = 0.3f;
    float fastSpeedMultiplier = 2.0f;
    
    // Constraints
    float minPitch = -89.0f;
    float maxPitch = 89.0f;
    float minDistance = 0.5f;
    float maxDistance = 100.0f;

public:
    CameraController() = default;
    
    /**
     * @brief Initialize with input system and camera
     */
    void init(InputSystem* input, rendering::Camera* cam) {
        inputSystem = input;
        camera = cam;
        
        if (camera) {
            position = camera->getPosition();
            // Calculate initial target point and distance for Trackball mode
            Vec3 cameraTarget = camera->getTarget();
            Vec3 toCamera = position - cameraTarget;
            distance = toCamera.length();
            targetPoint = cameraTarget;
            
            // Calculate initial angles
            if (distance > 0.001f) {
                yaw = atan2f(toCamera.z, toCamera.x) * 180.0f / 3.14159f;
                pitch = asinf(toCamera.y / distance) * 180.0f / 3.14159f;
            }
        }
    }
    
    /**
     * @brief Update camera based on input
     */
    void update(float deltaTime) {
        if (!inputSystem || !camera) return;
        
        switch (currentMode) {
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
    
    // ========== Mode Control ==========
    
    /**
     * @brief Set camera control mode
     */
    void setMode(Mode mode) {
        currentMode = mode;
        
        // Recalculate parameters when switching modes
        if (camera) {
            position = camera->getPosition();
            Vec3 cameraTarget = camera->getTarget();
            Vec3 toCamera = position - cameraTarget;
            distance = toCamera.length();
            targetPoint = cameraTarget;
            
            if (distance > 0.001f) {
                yaw = atan2f(toCamera.z, toCamera.x) * 180.0f / 3.14159f;
                pitch = asinf(toCamera.y / distance) * 180.0f / 3.14159f;
            }
        }
    }
    
    Mode getMode() const { return currentMode; }
    
    // ========== Target Point Control (for Trackball/Orbit modes) ==========
    
    void setTargetPoint(const Vec3& target) {
        targetPoint = target;
        if (camera) {
            Vec3 toCamera = position - targetPoint;
            distance = toCamera.length();
        }
    }
    
    Vec3 getTargetPoint() const { return targetPoint; }
    
    void setDistance(float dist) {
        distance = std::clamp(dist, minDistance, maxDistance);
    }
    
    float getDistance() const { return distance; }
    
    // ========== Getters/Setters ==========
    
    void setPosition(const Vec3& pos) {
        position = pos;
        if (camera) camera->setPosition(pos);
    }
    
    Vec3 getPosition() const { return position; }
    
    void setMoveSpeed(float speed) { moveSpeed = speed; }
    float getMoveSpeed() const { return moveSpeed; }
    
    void setPanSpeed(float speed) { panSpeed = speed; }
    float getPanSpeed() const { return panSpeed; }
    
    void setZoomSpeed(float speed) { zoomSpeed = speed; }
    float getZoomSpeed() const { return zoomSpeed; }
    
    void setRotationSensitivity(float sensitivity) { rotationSensitivity = sensitivity; }
    float getRotationSensitivity() const { return rotationSensitivity; }
    
    void setYaw(float y) { yaw = y; }
    float getYaw() const { return yaw; }
    
    void setPitch(float p) { 
        pitch = std::clamp(p, minPitch, maxPitch);
    }
    float getPitch() const { return pitch; }

private:
    // ========== Update Methods for Each Mode ==========
    
    /**
     * @brief Trackball mode: Spherical rotation around target
     */
    void updateTrackball(float deltaTime) {
        // Handle zoom (mouse wheel)
        float scroll = inputSystem->getMouseScrollDelta();
        if (fabsf(scroll) > 0.001f) {
            distance -= scroll * zoomSpeed;
            distance = std::clamp(distance, minDistance, maxDistance);
        }
        
        // Handle rotation (middle mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            yaw += static_cast<float>(dx) * rotationSensitivity;
            pitch -= static_cast<float>(dy) * rotationSensitivity;
            pitch = std::clamp(pitch, minPitch, maxPitch);
        }
        
        // Handle pan (right mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            Vec3 right = getRightVector();
            Vec3 up(0.0f, 1.0f, 0.0f);
            
            Vec3 panVector = right * static_cast<float>(-dx) * panSpeed * distance +
                            up * static_cast<float>(dy) * panSpeed * distance;
            
            targetPoint = targetPoint + panVector;
        }
        
        // Calculate camera position from spherical coordinates
        float yawRad = yaw * 3.14159f / 180.0f;
        float pitchRad = pitch * 3.14159f / 180.0f;
        
        position.x = targetPoint.x + distance * cosf(pitchRad) * cosf(yawRad);
        position.y = targetPoint.y + distance * sinf(pitchRad);
        position.z = targetPoint.z + distance * cosf(pitchRad) * sinf(yawRad);
        
        camera->setPosition(position);
        camera->setTarget(targetPoint);
    }
    
    /**
     * @brief Orbit mode: Cylindrical rotation around target
     */
    void updateOrbit(float deltaTime) {
        // Handle zoom (mouse wheel)
        float scroll = inputSystem->getMouseScrollDelta();
        if (fabsf(scroll) > 0.001f) {
            distance -= scroll * zoomSpeed;
            distance = std::clamp(distance, minDistance, maxDistance);
        }
        
        // Handle rotation (middle mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            yaw += static_cast<float>(dx) * rotationSensitivity;
            pitch -= static_cast<float>(dy) * rotationSensitivity;
            pitch = std::clamp(pitch, minPitch, maxPitch);
        }
        
        // Handle pan (right mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            Vec3 right = getRightVector();
            Vec3 up(0.0f, 1.0f, 0.0f);
            
            Vec3 panVector = right * static_cast<float>(-dx) * panSpeed * distance +
                            up * static_cast<float>(dy) * panSpeed * distance;
            
            targetPoint = targetPoint + panVector;
        }
        
        // Calculate camera position from cylindrical coordinates
        // Orbit maintains constant Y-level rotation (horizontal only for yaw)
        float yawRad = yaw * 3.14159f / 180.0f;
        float pitchRad = pitch * 3.14159f / 180.0f;
        
        float horizontalDist = distance * cosf(pitchRad);
        
        position.x = targetPoint.x + horizontalDist * cosf(yawRad);
        position.y = targetPoint.y + distance * sinf(pitchRad);
        position.z = targetPoint.z + horizontalDist * sinf(yawRad);
        
        camera->setPosition(position);
        camera->setTarget(targetPoint);
    }
    
    /**
     * @brief First-Person mode: FPS-style movement
     */
    void updateFirstPerson(float deltaTime) {
        // Handle rotation (middle mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            yaw += static_cast<float>(dx) * rotationSensitivity;
            pitch -= static_cast<float>(dy) * rotationSensitivity;
            pitch = std::clamp(pitch, minPitch, maxPitch);
        }
        
        // Calculate forward, right, up vectors
        Vec3 forward = getForwardVector();
        Vec3 right = getRightVector();
        Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
        
        // Movement with WASD
        float speed = moveSpeed;
        if (inputSystem->isKeyDown(KeyCode::LeftShift)) {
            speed *= fastSpeedMultiplier;
        }
        
        Vec3 velocity(0.0f, 0.0f, 0.0f);
        
        if (inputSystem->isKeyDown(KeyCode::W)) {
            velocity = velocity + forward;
        }
        if (inputSystem->isKeyDown(KeyCode::S)) {
            velocity = velocity - forward;
        }
        if (inputSystem->isKeyDown(KeyCode::A)) {
            velocity = velocity - right;
        }
        if (inputSystem->isKeyDown(KeyCode::D)) {
            velocity = velocity + right;
        }
        if (inputSystem->isKeyDown(KeyCode::Space)) {
            velocity = velocity + up;
        }
        if (inputSystem->isKeyDown(KeyCode::LeftControl)) {
            velocity = velocity - up;
        }
        
        // Normalize and apply speed
        float length = velocity.length();
        if (length > 0.001f) {
            velocity = velocity * (speed * deltaTime / length);
            position = position + velocity;
        }
        
        // Handle pan (right mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            Vec3 panVector = right * static_cast<float>(-dx) * panSpeed * 10.0f +
                            up * static_cast<float>(dy) * panSpeed * 10.0f;
            
            position = position + panVector;
        }
        
        camera->setPosition(position);
        Vec3 target = position + forward;
        camera->setTarget(target);
    }
    
    /**
     * @brief Free mode: Free-flying camera
     */
    void updateFree(float deltaTime) {
        // Handle rotation (middle mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Middle)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            yaw += static_cast<float>(dx) * rotationSensitivity;
            pitch -= static_cast<float>(dy) * rotationSensitivity;
            pitch = std::clamp(pitch, minPitch, maxPitch);
        }
        
        // Calculate forward, right, up vectors
        Vec3 forward = getForwardVector();
        Vec3 right = getRightVector();
        Vec3 up(0.0f, 1.0f, 0.0f);
        
        // Free movement with WASD (camera direction-based)
        float speed = moveSpeed;
        if (inputSystem->isKeyDown(KeyCode::LeftShift)) {
            speed *= fastSpeedMultiplier;
        }
        
        Vec3 velocity(0.0f, 0.0f, 0.0f);
        
        if (inputSystem->isKeyDown(KeyCode::W)) {
            velocity = velocity + forward;
        }
        if (inputSystem->isKeyDown(KeyCode::S)) {
            velocity = velocity - forward;
        }
        if (inputSystem->isKeyDown(KeyCode::A)) {
            velocity = velocity - right;
        }
        if (inputSystem->isKeyDown(KeyCode::D)) {
            velocity = velocity + right;
        }
        if (inputSystem->isKeyDown(KeyCode::Space)) {
            velocity = velocity + up;
        }
        if (inputSystem->isKeyDown(KeyCode::LeftControl)) {
            velocity = velocity - up;
        }
        
        // Normalize and apply speed
        float length = velocity.length();
        if (length > 0.001f) {
            velocity = velocity * (speed * deltaTime / length);
            position = position + velocity;
        }
        
        // Handle pan (right mouse button drag)
        if (inputSystem->isMouseButtonDown(MouseButton::Right)) {
            double dx, dy;
            inputSystem->getMouseDelta(dx, dy);
            
            Vec3 panVector = right * static_cast<float>(-dx) * panSpeed * 10.0f +
                            up * static_cast<float>(dy) * panSpeed * 10.0f;
            
            position = position + panVector;
        }
        
        camera->setPosition(position);
        Vec3 target = position + forward;
        camera->setTarget(target);
    }
    
    // ========== Helper Methods ==========
    
    Vec3 getForwardVector() const {
        // Convert yaw/pitch to direction vector
        float yawRad = yaw * 3.14159f / 180.0f;
        float pitchRad = pitch * 3.14159f / 180.0f;
        
        Vec3 forward;
        forward.x = cosf(pitchRad) * cosf(yawRad);
        forward.y = sinf(pitchRad);
        forward.z = cosf(pitchRad) * sinf(yawRad);
        
        float length = forward.length();
        if (length > 0.001f) {
            forward = forward * (1.0f / length);
        }
        
        return forward;
    }
    
    Vec3 getRightVector() const {
        Vec3 forward = getForwardVector();
        Vec3 up(0.0f, 1.0f, 0.0f);
        
        // Right = forward × up
        Vec3 right;
        right.x = forward.y * up.z - forward.z * up.y;
        right.y = forward.z * up.x - forward.x * up.z;
        right.z = forward.x * up.y - forward.y * up.x;
        
        float length = right.length();
        if (length > 0.001f) {
            right = right * (1.0f / length);
        }
        
        return right;
    }
};

} // namespace rs_engine
