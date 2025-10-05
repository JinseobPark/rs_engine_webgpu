#pragma once

#include "../../core/math/Vec3.h"
#include "../../core/math/Quat.h"

namespace rs_engine {

// Forward declarations
class InputSystem;
enum class KeyCode;
enum class MouseButton;

namespace rendering {
    class Camera;
}

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
    
    // Trackball/Orbit parameters (Quaternion-based)
    Vec3 target;                    // Look-at target point
    float distance;                 // Distance from target
    Quat orientation;               // Camera orientation (no gimbal lock!)
    
    // FirstPerson/Free parameters (Quaternion-based)
    Quat firstPersonOrientation;    // FPS camera orientation
    
    // Initial state for reset
    Vec3 initialPosition;           // Initial camera position
    Vec3 initialTarget;             // Initial target position
    float initialDistance;          // Initial distance
    Quat initialOrientation;        // Initial orientation
    
    // Control parameters
    float panSpeed = 1.0f;
    float zoomSpeed = 1.0f;
    float rotationSpeed = 0.3f;     // Reduced for smoother quaternion rotation
    float firstPersonMoveSpeed = 5.0f;
    
    // Constraints
    float minDistance = 0.5f;
    float maxDistance = 100.0f;

public:
    CameraController() = default;
    
    void init(InputSystem* inputSys, rendering::Camera* cam);
    void update(float deltaTime);
    
    // Mode control
    void setMode(Mode mode);
    Mode getMode() const { return currentMode; }
    
    // Target control (for Trackball/Orbit)
    void setTarget(const Vec3& newTarget);
    Vec3 getTarget() const { return target; }
    
    void setDistance(float dist);
    float getDistance() const { return distance; }
    
    // Speed/sensitivity settings
    void setPanSpeed(float speed) { panSpeed = speed; }
    float getPanSpeed() const { return panSpeed; }
    
    void setZoomSpeed(float speed) { zoomSpeed = speed; }
    float getZoomSpeed() const { return zoomSpeed; }
    
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    float getRotationSpeed() const { return rotationSpeed; }
    
    void setFirstPersonMoveSpeed(float speed) { firstPersonMoveSpeed = speed; }
    float getFirstPersonMoveSpeed() const { return firstPersonMoveSpeed; }
    
    // Reset camera to initial state
    void reset();

private:
    void updateTrackball(float deltaTime);
    void updateOrbit(float deltaTime);
    void updateFirstPerson(float deltaTime);
    void updateFree(float deltaTime);
};

} // namespace rs_engine
