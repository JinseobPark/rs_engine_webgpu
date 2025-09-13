#pragma once

#include "../core/Application.h"
#include <GLFW/glfw3.h>

namespace rs_engine {

class NativeApplication : public Application {
private:
    GLFWwindow* window = nullptr;
    
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    bool initPlatform() override;
    bool initWebGPU() override;
    void handleEvents() override;
    void cleanup() override;
};

} // namespace rs_engine
