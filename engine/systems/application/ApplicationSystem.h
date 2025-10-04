#pragma once

#include "../../core/IEngineSystem.h"
#include "../../core/Config.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #include <webgpu/webgpu.h>
    #include <webgpu/webgpu_cpp.h>
    #include <emscripten/html5_webgpu.h>
#else
    #include <GLFW/glfw3.h>
    #include <dawn/dawn_proc.h>
    #include <dawn/native/DawnNative.h>
    #include <dawn/webgpu_cpp.h>
    #include <webgpu/webgpu_glfw.h>
#endif

namespace rs_engine {

/**
 * @brief Application System - Platform abstraction and WebGPU initialization
 * 
 * Responsibilities:
 * - Window/Canvas management
 * - Event handling
 * - WebGPU device initialization
 * - Surface configuration
 * 
 * Platform Support: 95% shared, 5% platform-specific initialization
 */
class ApplicationSystem : public IEngineSystem {
protected:
    // Common WebGPU resources
    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Surface surface = nullptr;

    // Common state
    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
    bool shouldCloseFlag = false;

    // Platform limits
    PlatformLimits platformLimits = EngineConfig::getLimits();

#ifndef __EMSCRIPTEN__
    // Native-specific
    GLFWwindow* window = nullptr;
    static bool s_glfwInitialized;
    std::unique_ptr<dawn::native::Instance> dawnInstance;
#endif

public:
    virtual ~ApplicationSystem() = default;

    // IEngineSystem interface
    bool initialize(Engine* engineRef) override;
    void onStart() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    const char* getName() const override { return "Application"; }
    int getPriority() const override { return -100; }

    // Platform-specific initialization
    virtual bool initPlatform();
    virtual bool initWebGPU();
    virtual void handleEvents();
    virtual void configureSurface();

    // Accessors
    wgpu::Device& getDevice() { return device; }
    wgpu::Surface& getSurface() { return surface; }
    wgpu::Instance& getInstance() { return instance; }
    
    uint32_t getWindowWidth() const { return windowWidth; }
    uint32_t getWindowHeight() const { return windowHeight; }
    
    bool shouldClose() const { return shouldCloseFlag; }
    void setShouldClose(bool value) { shouldCloseFlag = value; }

#ifndef __EMSCRIPTEN__
    GLFWwindow* getWindow() { return window; }
    void onWindowResize(int width, int height);
    
    // GLFW callbacks
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
#endif
};

} // namespace rs_engine
