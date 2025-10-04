#pragma once

#include "../../core/IEngineSystem.h"
#include "CameraController.h"
#include <unordered_map>
#include <array>
#include <memory>

#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
#endif

namespace rs_engine {

// Forward declarations
namespace rendering {
    class Camera;
}

/**
 * @brief Input state for keys and mouse buttons
 */
enum class InputState {
    Released,  // Not pressed
    Pressed,   // Just pressed this frame
    Held,      // Held down (pressed for multiple frames)
    JustReleased // Just released this frame
};

/**
 * @brief Key codes (unified for Web and Native)
 */
enum class KeyCode {
    // Letters
    A = 0, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // Special keys
    Escape, Tab, CapsLock, Shift, Control, Alt, Space, Enter, Backspace,
    LeftShift, RightShift, LeftControl, RightControl, LeftAlt, RightAlt,
    
    // Arrow keys
    Up, Down, Left, Right,
    
    // Editing
    Insert, Delete, Home, End, PageUp, PageDown,
    
    // Count
    KeyCount
};

/**
 * @brief Mouse button codes
 */
enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    
    ButtonCount
};

/**
 * @brief Input System - Unified input handling for keyboard and mouse
 * 
 * Responsibilities:
 * - Keyboard input state tracking
 * - Mouse input state tracking
 * - Mouse position and delta tracking
 * - Cross-platform input abstraction
 * 
 * Platform Support: 100% shared API, platform-specific input sources
 */
class InputSystem : public IEngineSystem {
private:
    // Camera controller
    std::unique_ptr<CameraController> cameraController;
    
    // Keyboard state
    std::array<InputState, static_cast<size_t>(KeyCode::KeyCount)> keyStates;
    std::array<InputState, static_cast<size_t>(KeyCode::KeyCount)> prevKeyStates;
    
    // Mouse state
    std::array<InputState, static_cast<size_t>(MouseButton::ButtonCount)> mouseButtonStates;
    std::array<InputState, static_cast<size_t>(MouseButton::ButtonCount)> prevMouseButtonStates;
    
    // Mouse position
    double mouseX = 0.0;
    double mouseY = 0.0;
    double mouseDeltaX = 0.0;
    double mouseDeltaY = 0.0;
    double prevMouseX = 0.0;
    double prevMouseY = 0.0;
    
    // Mouse scroll
    double scrollDeltaX = 0.0;
    double scrollDeltaY = 0.0;
    
    // Input capture state
    bool cursorLocked = false;
    bool cursorVisible = true;

public:
    InputSystem();
    virtual ~InputSystem() = default;

    // IEngineSystem interface
    bool initialize(Engine* engineRef) override;
    void onStart() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    const char* getName() const override { return "Input"; }
    int getPriority() const override { return -50; } // After Application, before gameplay

    // ========== Keyboard Input ==========
    
    /**
     * @brief Check if key was just pressed this frame
     */
    bool isKeyPressed(KeyCode key) const;
    
    /**
     * @brief Check if key is currently held down
     */
    bool isKeyHeld(KeyCode key) const;
    
    /**
     * @brief Check if key was just released this frame
     */
    bool isKeyReleased(KeyCode key) const;
    
    /**
     * @brief Check if key is down (pressed or held)
     */
    bool isKeyDown(KeyCode key) const;

    // ========== Mouse Input ==========
    
    /**
     * @brief Check if mouse button was just pressed this frame
     */
    bool isMouseButtonPressed(MouseButton button) const;
    
    /**
     * @brief Check if mouse button is currently held down
     */
    bool isMouseButtonHeld(MouseButton button) const;
    
    /**
     * @brief Check if mouse button was just released this frame
     */
    bool isMouseButtonReleased(MouseButton button) const;
    
    /**
     * @brief Check if mouse button is down (pressed or held)
     */
    bool isMouseButtonDown(MouseButton button) const;

    // ========== Mouse Position ==========
    
    /**
     * @brief Get current mouse position
     */
    void getMousePosition(double& x, double& y) const;
    
    /**
     * @brief Get mouse movement delta since last frame
     */
    void getMouseDelta(double& dx, double& dy) const;
    
    /**
     * @brief Get mouse scroll delta
     */
    void getScrollDelta(double& dx, double& dy) const;
    
    /**
     * @brief Get vertical mouse scroll delta (convenience method)
     * @return Scroll delta Y value (positive = scroll up, negative = scroll down)
     */
    float getMouseScrollDelta() const { return static_cast<float>(scrollDeltaY); }

    // ========== Mouse Control ==========
    
    /**
     * @brief Lock cursor to window center (for FPS camera)
     */
    void lockCursor(bool lock);
    
    /**
     * @brief Show or hide cursor
     */
    void showCursor(bool show);
    
    /**
     * @brief Check if cursor is locked
     */
    bool isCursorLocked() const { return cursorLocked; }
    
    /**
     * @brief Check if cursor is visible
     */
    bool isCursorVisible() const { return cursorVisible; }

    // ========== Camera Controller ==========
    
    /**
     * @brief Initialize camera controller with a camera
     * @param camera Camera to control (owned by Scene)
     */
    void initializeCameraController(rendering::Camera* camera);
    
    /**
     * @brief Get camera controller
     */
    CameraController* getCameraController() { return cameraController.get(); }

    // ========== Internal Update (called by platform) ==========
    
    /**
     * @brief Called by ApplicationSystem to update key state
     * @param key Platform-specific key code
     * @param pressed true if pressed, false if released
     */
    void updateKeyState(int platformKey, bool pressed);
    
    /**
     * @brief Called by ApplicationSystem to update mouse button state
     * @param button Platform-specific button code
     * @param pressed true if pressed, false if released
     */
    void updateMouseButtonState(int platformButton, bool pressed);
    
    /**
     * @brief Called by ApplicationSystem to update mouse position
     */
    void updateMousePosition(double x, double y);
    
    /**
     * @brief Called by ApplicationSystem to update scroll
     */
    void updateScroll(double dx, double dy);

private:
    /**
     * @brief Update input states for new frame
     */
    void updateStates();
    
    /**
     * @brief Convert platform key code to KeyCode
     */
    KeyCode platformKeyToKeyCode(int platformKey) const;
    
    /**
     * @brief Convert platform mouse button to MouseButton
     */
    MouseButton platformButtonToMouseButton(int platformButton) const;

#ifdef __EMSCRIPTEN__
    // Web event handlers
    void setupWebEventHandlers();
    
    static EM_BOOL onMouseDown(int eventType, const EmscriptenMouseEvent* event, void* userData);
    static EM_BOOL onMouseUp(int eventType, const EmscriptenMouseEvent* event, void* userData);
    static EM_BOOL onMouseMove(int eventType, const EmscriptenMouseEvent* event, void* userData);
    static EM_BOOL onWheel(int eventType, const EmscriptenWheelEvent* event, void* userData);
    static EM_BOOL onKeyDown(int eventType, const EmscriptenKeyboardEvent* event, void* userData);
    static EM_BOOL onKeyUp(int eventType, const EmscriptenKeyboardEvent* event, void* userData);
#endif
};

} // namespace rs_engine
