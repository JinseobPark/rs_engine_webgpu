#pragma once

#include "../core/Application.h"
#include <GLFW/glfw3.h>

namespace rs_engine {

class NativeApplication : public Application {
private:
    GLFWwindow* window = nullptr;
    static bool s_glfwInitialized;
    
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

public:
    bool initPlatform() override;
    bool initWebGPU() override;
    void handleEvents() override;
    void cleanup() override;
    GLFWwindow* getWindow() override { return window; }
    void onWindowResize(int width, int height);
    
    // Static method to check if GLFW is still initialized
    static bool isGLFWInitialized() { return s_glfwInitialized; }
};

} // namespace rs_engine
