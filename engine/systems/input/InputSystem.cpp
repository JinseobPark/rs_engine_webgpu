#include "InputSystem.h"
#include "../../core/Engine.h"
#include "../application/ApplicationSystem.h"
#include "../../rendering/scene/Camera.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
#else
    #include <GLFW/glfw3.h>
#endif

namespace rs_engine {

InputSystem::InputSystem() {
    // Initialize all states to Released
    keyStates.fill(InputState::Released);
    prevKeyStates.fill(InputState::Released);
    mouseButtonStates.fill(InputState::Released);
    prevMouseButtonStates.fill(InputState::Released);
}

bool InputSystem::initialize(Engine* engineRef) {
    if (!IEngineSystem::initialize(engineRef)) {
        return false;
    }

    std::cout << "[INFO] Initializing Input System..." << std::endl;
    std::cout << "[SUCCESS] Input System initialized" << std::endl;
    return true;
}

void InputSystem::onStart() {
    std::cout << "[Input] Started - Keyboard and mouse tracking enabled" << std::endl;
}

void InputSystem::onUpdate(float deltaTime) {
    // Update input states (Pressed -> Held, JustReleased -> Released)
    updateStates();
    
    // Update mouse delta
    mouseDeltaX = mouseX - prevMouseX;
    mouseDeltaY = mouseY - prevMouseY;
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    
    // Update camera controller BEFORE resetting scroll
    // (camera controller needs to read scroll delta)
    if (cameraController) {
        cameraController->update(deltaTime);
    }
    
    // Reset scroll delta AFTER camera controller uses it
    // (scroll is per-frame, only valid for one frame)
    scrollDeltaX = 0.0;
    scrollDeltaY = 0.0;
}

void InputSystem::onShutdown() {
    std::cout << "[Input] Shutting down..." << std::endl;
}

// ========== Keyboard Input ==========

bool InputSystem::isKeyPressed(KeyCode key) const {
    size_t index = static_cast<size_t>(key);
    if (index >= keyStates.size()) return false;
    return keyStates[index] == InputState::Pressed;
}

bool InputSystem::isKeyHeld(KeyCode key) const {
    size_t index = static_cast<size_t>(key);
    if (index >= keyStates.size()) return false;
    return keyStates[index] == InputState::Held;
}

bool InputSystem::isKeyReleased(KeyCode key) const {
    size_t index = static_cast<size_t>(key);
    if (index >= keyStates.size()) return false;
    return keyStates[index] == InputState::JustReleased;
}

bool InputSystem::isKeyDown(KeyCode key) const {
    size_t index = static_cast<size_t>(key);
    if (index >= keyStates.size()) return false;
    return keyStates[index] == InputState::Pressed || keyStates[index] == InputState::Held;
}

// ========== Mouse Input ==========

bool InputSystem::isMouseButtonPressed(MouseButton button) const {
    size_t index = static_cast<size_t>(button);
    if (index >= mouseButtonStates.size()) return false;
    return mouseButtonStates[index] == InputState::Pressed;
}

bool InputSystem::isMouseButtonHeld(MouseButton button) const {
    size_t index = static_cast<size_t>(button);
    if (index >= mouseButtonStates.size()) return false;
    return mouseButtonStates[index] == InputState::Held;
}

bool InputSystem::isMouseButtonReleased(MouseButton button) const {
    size_t index = static_cast<size_t>(button);
    if (index >= mouseButtonStates.size()) return false;
    return mouseButtonStates[index] == InputState::JustReleased;
}

bool InputSystem::isMouseButtonDown(MouseButton button) const {
    size_t index = static_cast<size_t>(button);
    if (index >= mouseButtonStates.size()) return false;
    return mouseButtonStates[index] == InputState::Pressed || 
           mouseButtonStates[index] == InputState::Held;
}

// ========== Mouse Position ==========

void InputSystem::getMousePosition(double& x, double& y) const {
    x = mouseX;
    y = mouseY;
}

void InputSystem::getMouseDelta(double& dx, double& dy) const {
    dx = mouseDeltaX;
    dy = -mouseDeltaY;
}

void InputSystem::getScrollDelta(double& dx, double& dy) const {
    dx = scrollDeltaX;
    dy = scrollDeltaY;
}

// ========== Mouse Control ==========

void InputSystem::lockCursor(bool lock) {
    cursorLocked = lock;
    
#ifndef __EMSCRIPTEN__
    // Get window from ApplicationSystem
    auto* appSystem = engine->getSystem<ApplicationSystem>();
    if (appSystem && appSystem->getWindow()) {
        if (lock) {
            glfwSetInputMode(appSystem->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(appSystem->getWindow(), GLFW_CURSOR, 
                           cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
        }
    }
#else
    // Web: Use Pointer Lock API (to be implemented via JS)
    std::cout << "[Input] Cursor lock " << (lock ? "enabled" : "disabled") 
              << " (Web implementation needed)" << std::endl;
#endif
}

void InputSystem::showCursor(bool show) {
    cursorVisible = show;
    
#ifndef __EMSCRIPTEN__
    auto* appSystem = engine->getSystem<ApplicationSystem>();
    if (appSystem && appSystem->getWindow() && !cursorLocked) {
        glfwSetInputMode(appSystem->getWindow(), GLFW_CURSOR,
                       show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
#else
    // Web: CSS cursor style
    std::cout << "[Input] Cursor " << (show ? "shown" : "hidden")
              << " (Web implementation needed)" << std::endl;
#endif
}

// ========== Internal Update ==========

void InputSystem::updateKeyState(int platformKey, bool pressed) {
    KeyCode key = platformKeyToKeyCode(platformKey);
    size_t index = static_cast<size_t>(key);
    
    if (index >= keyStates.size()) return;
    
    if (pressed) {
        // Only set to Pressed if it was Released
        if (keyStates[index] == InputState::Released || 
            keyStates[index] == InputState::JustReleased) {
            keyStates[index] = InputState::Pressed;
        }
    } else {
        keyStates[index] = InputState::JustReleased;
    }
}

void InputSystem::updateMouseButtonState(int platformButton, bool pressed) {
    MouseButton button = platformButtonToMouseButton(platformButton);
    size_t index = static_cast<size_t>(button);
    
    if (index >= mouseButtonStates.size()) return;
    
    if (pressed) {
        if (mouseButtonStates[index] == InputState::Released ||
            mouseButtonStates[index] == InputState::JustReleased) {
            mouseButtonStates[index] = InputState::Pressed;
        }
    } else {
        mouseButtonStates[index] = InputState::JustReleased;
    }
}

void InputSystem::updateMousePosition(double x, double y) {
    mouseX = x;
    mouseY = y;
}

void InputSystem::updateScroll(double dx, double dy) {
    scrollDeltaX = dx;
    scrollDeltaY = dy;
}

// ========== Private Methods ==========

void InputSystem::updateStates() {
    // Update keyboard states
    for (size_t i = 0; i < keyStates.size(); ++i) {
        if (keyStates[i] == InputState::Pressed) {
            keyStates[i] = InputState::Held;
        } else if (keyStates[i] == InputState::JustReleased) {
            keyStates[i] = InputState::Released;
        }
    }
    
    // Update mouse button states
    for (size_t i = 0; i < mouseButtonStates.size(); ++i) {
        if (mouseButtonStates[i] == InputState::Pressed) {
            mouseButtonStates[i] = InputState::Held;
        } else if (mouseButtonStates[i] == InputState::JustReleased) {
            mouseButtonStates[i] = InputState::Released;
        }
    }
}

KeyCode InputSystem::platformKeyToKeyCode(int platformKey) const {
#ifndef __EMSCRIPTEN__
    // GLFW key codes
    switch (platformKey) {
        // Letters
        case GLFW_KEY_A: return KeyCode::A;
        case GLFW_KEY_B: return KeyCode::B;
        case GLFW_KEY_C: return KeyCode::C;
        case GLFW_KEY_D: return KeyCode::D;
        case GLFW_KEY_E: return KeyCode::E;
        case GLFW_KEY_F: return KeyCode::F;
        case GLFW_KEY_G: return KeyCode::G;
        case GLFW_KEY_H: return KeyCode::H;
        case GLFW_KEY_I: return KeyCode::I;
        case GLFW_KEY_J: return KeyCode::J;
        case GLFW_KEY_K: return KeyCode::K;
        case GLFW_KEY_L: return KeyCode::L;
        case GLFW_KEY_M: return KeyCode::M;
        case GLFW_KEY_N: return KeyCode::N;
        case GLFW_KEY_O: return KeyCode::O;
        case GLFW_KEY_P: return KeyCode::P;
        case GLFW_KEY_Q: return KeyCode::Q;
        case GLFW_KEY_R: return KeyCode::R;
        case GLFW_KEY_S: return KeyCode::S;
        case GLFW_KEY_T: return KeyCode::T;
        case GLFW_KEY_U: return KeyCode::U;
        case GLFW_KEY_V: return KeyCode::V;
        case GLFW_KEY_W: return KeyCode::W;
        case GLFW_KEY_X: return KeyCode::X;
        case GLFW_KEY_Y: return KeyCode::Y;
        case GLFW_KEY_Z: return KeyCode::Z;
        
        // Numbers
        case GLFW_KEY_0: return KeyCode::Num0;
        case GLFW_KEY_1: return KeyCode::Num1;
        case GLFW_KEY_2: return KeyCode::Num2;
        case GLFW_KEY_3: return KeyCode::Num3;
        case GLFW_KEY_4: return KeyCode::Num4;
        case GLFW_KEY_5: return KeyCode::Num5;
        case GLFW_KEY_6: return KeyCode::Num6;
        case GLFW_KEY_7: return KeyCode::Num7;
        case GLFW_KEY_8: return KeyCode::Num8;
        case GLFW_KEY_9: return KeyCode::Num9;
        
        // Function keys
        case GLFW_KEY_F1: return KeyCode::F1;
        case GLFW_KEY_F2: return KeyCode::F2;
        case GLFW_KEY_F3: return KeyCode::F3;
        case GLFW_KEY_F4: return KeyCode::F4;
        case GLFW_KEY_F5: return KeyCode::F5;
        case GLFW_KEY_F6: return KeyCode::F6;
        case GLFW_KEY_F7: return KeyCode::F7;
        case GLFW_KEY_F8: return KeyCode::F8;
        case GLFW_KEY_F9: return KeyCode::F9;
        case GLFW_KEY_F10: return KeyCode::F10;
        case GLFW_KEY_F11: return KeyCode::F11;
        case GLFW_KEY_F12: return KeyCode::F12;
        
        // Special keys
        case GLFW_KEY_ESCAPE: return KeyCode::Escape;
        case GLFW_KEY_TAB: return KeyCode::Tab;
        case GLFW_KEY_CAPS_LOCK: return KeyCode::CapsLock;
        case GLFW_KEY_LEFT_SHIFT: return KeyCode::LeftShift;
        case GLFW_KEY_RIGHT_SHIFT: return KeyCode::RightShift;
        case GLFW_KEY_LEFT_CONTROL: return KeyCode::LeftControl;
        case GLFW_KEY_RIGHT_CONTROL: return KeyCode::RightControl;
        case GLFW_KEY_LEFT_ALT: return KeyCode::LeftAlt;
        case GLFW_KEY_RIGHT_ALT: return KeyCode::RightAlt;
        case GLFW_KEY_SPACE: return KeyCode::Space;
        case GLFW_KEY_ENTER: return KeyCode::Enter;
        case GLFW_KEY_BACKSPACE: return KeyCode::Backspace;
        
        // Arrow keys
        case GLFW_KEY_UP: return KeyCode::Up;
        case GLFW_KEY_DOWN: return KeyCode::Down;
        case GLFW_KEY_LEFT: return KeyCode::Left;
        case GLFW_KEY_RIGHT: return KeyCode::Right;
        
        // Editing
        case GLFW_KEY_INSERT: return KeyCode::Insert;
        case GLFW_KEY_DELETE: return KeyCode::Delete;
        case GLFW_KEY_HOME: return KeyCode::Home;
        case GLFW_KEY_END: return KeyCode::End;
        case GLFW_KEY_PAGE_UP: return KeyCode::PageUp;
        case GLFW_KEY_PAGE_DOWN: return KeyCode::PageDown;
        
        default: return KeyCode::KeyCount; // Invalid
    }
#else
    // Web: HTML5 key codes (to be implemented)
    // For now, return invalid
    return KeyCode::KeyCount;
#endif
}

MouseButton InputSystem::platformButtonToMouseButton(int platformButton) const {
#ifndef __EMSCRIPTEN__
    // GLFW mouse buttons
    switch (platformButton) {
        case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4: return MouseButton::Button4;
        case GLFW_MOUSE_BUTTON_5: return MouseButton::Button5;
        default: return MouseButton::ButtonCount; // Invalid
    }
#else
    // Web: HTML5 mouse buttons
    return static_cast<MouseButton>(platformButton);
#endif
}

// ========== Camera Controller ==========

void InputSystem::initializeCameraController(rendering::Camera* camera) {
    if (!camera) {
        std::cerr << "[InputSystem] Cannot initialize camera controller: camera is null" << std::endl;
        return;
    }
    
    if (!cameraController) {
        cameraController = std::make_unique<CameraController>();
    }
    
    cameraController->init(this, camera);
    cameraController->setMode(CameraController::Mode::Trackball);
    cameraController->setTarget(Vec3(0.0f, 0.0f, 0.0f));
    
    std::cout << "[InputSystem] Camera controller initialized (Trackball mode)" << std::endl;
    
#ifdef __EMSCRIPTEN__
    // Setup Web event handlers
    setupWebEventHandlers();
#endif
}

#ifdef __EMSCRIPTEN__
// ========== Web Event Handlers ==========

namespace {
    /**
     * @brief Convert HTML5 mouse button code to our MouseButton enum
     * 
     * HTML5 Standard:     Our MouseButton enum:
     * 0 = Left Button     0 = Left
     * 1 = Middle Button   1 = Right
     * 2 = Right Button    2 = Middle
     * 
     * We need to swap Middle(1) and Right(2) to match our enum
     */
    inline int convertHTML5Button(int html5Button) {
        switch (html5Button) {
            case 0: return 0;  // Left → Left (no change)
            case 1: return 2;  // HTML5 Middle(1) → Our Middle(2)
            case 2: return 1;  // HTML5 Right(2) → Our Right(1)
            default: return html5Button;
        }
    }
}

EM_BOOL InputSystem::onMouseDown(int eventType, const EmscriptenMouseEvent* event, void* userData) {
    auto* inputSystem = static_cast<InputSystem*>(userData);
    if (!inputSystem) return EM_FALSE;
    
    // Convert HTML5 button code to our MouseButton enum
    int button = convertHTML5Button(event->button);
    inputSystem->updateMouseButtonState(button, true);
    
    return EM_TRUE;
}

EM_BOOL InputSystem::onMouseUp(int eventType, const EmscriptenMouseEvent* event, void* userData) {
    auto* inputSystem = static_cast<InputSystem*>(userData);
    if (!inputSystem) return EM_FALSE;
    
    // Convert HTML5 button code to our MouseButton enum
    int button = convertHTML5Button(event->button);
    inputSystem->updateMouseButtonState(button, false);
    
    return EM_TRUE;
}

EM_BOOL InputSystem::onMouseMove(int eventType, const EmscriptenMouseEvent* event, void* userData) {
    auto* inputSystem = static_cast<InputSystem*>(userData);
    if (!inputSystem) return EM_FALSE;
    
    inputSystem->updateMousePosition(event->targetX, event->targetY);
    
    return EM_TRUE;
}

EM_BOOL InputSystem::onWheel(int eventType, const EmscriptenWheelEvent* event, void* userData) {
    auto* inputSystem = static_cast<InputSystem*>(userData);
    if (!inputSystem) return EM_FALSE;
    
    inputSystem->updateScroll(event->deltaX, event->deltaY);
    
    return EM_TRUE;
}

EM_BOOL InputSystem::onKeyDown(int eventType, const EmscriptenKeyboardEvent* event, void* userData) {
    auto* inputSystem = static_cast<InputSystem*>(userData);
    if (!inputSystem) return EM_FALSE;
    
    // Convert HTML5 key code to our KeyCode enum
    // This is simplified - you may need more mappings
    int keyCode = 0; // TODO: Map event->key or event->keyCode to KeyCode
    inputSystem->updateKeyState(keyCode, true);
    
    return EM_TRUE;
}

EM_BOOL InputSystem::onKeyUp(int eventType, const EmscriptenKeyboardEvent* event, void* userData) {
    auto* inputSystem = static_cast<InputSystem*>(userData);
    if (!inputSystem) return EM_FALSE;
    
    int keyCode = 0; // TODO: Map event->key or event->keyCode to KeyCode
    inputSystem->updateKeyState(keyCode, false);
    
    return EM_TRUE;
}

void InputSystem::setupWebEventHandlers() {
    const char* target = "#canvas";
    
    emscripten_set_mousedown_callback(target, this, true, onMouseDown);
    emscripten_set_mouseup_callback(target, this, true, onMouseUp);
    emscripten_set_mousemove_callback(target, this, true, onMouseMove);
    emscripten_set_wheel_callback(target, this, true, onWheel);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, onKeyDown);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, onKeyUp);
    
    std::cout << "[InputSystem] Web event handlers registered" << std::endl;
}

#endif

} // namespace rs_engine
