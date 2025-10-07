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
        RSEngine,     // Default: Maya-style combined rotation (like old RSCamera)
        Trackball,    // Spherical rotation (like 3D modeling tools)
        Orbit,        // Cylindrical rotation (horizontal + vertical)
        FirstPerson,  // FPS-style movement
        Free          // Free-flying camera
    };

private:
    InputSystem* inputSystem = nullptr;
    rendering::Camera* camera = nullptr;
    
    // Current mode
    Mode currentMode = Mode::RSEngine;
    
    // FirstPerson/Free parameters (Quaternion-based)
    // This is the ONLY orientation state we store, used exclusively for FPS/Free modes
    Quat firstPersonOrientation;    // FPS camera orientation
    
    // Rotation axis for RS Engine mode
    Vec3 rotateAxis = Vec3(0.0f, 0.0f, 0.0f);
    
    // Focal length (RSCamera style)
    float focalLength = 10.0f;
    
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
    Vec3 getTarget() const;
    
    // Distance is computed from camera position and target
    float getDistance() const;
    void setDistance(float dist);
    
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
    void updateRSEngine(float deltaTime);
    void updateTrackball(float deltaTime);
    void updateOrbit(float deltaTime);
    void updateFirstPerson(float deltaTime);
    void updateFree(float deltaTime);
    
    // Helper methods to compute orientation when needed
    Quat getCurrentOrientation() const;
    
    // RSCamera style: Orthogonalize up vector after rotation
    void updateCameraVectors();
};

} // namespace rs_engine
