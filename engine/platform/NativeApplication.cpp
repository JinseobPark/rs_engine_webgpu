#include "NativeApplication.h"
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <webgpu/webgpu_glfw.h>
#include <vector>
#include <memory>

namespace rs_engine {

void NativeApplication::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void NativeApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    
    // Get application instance from window user pointer
    NativeApplication* app = static_cast<NativeApplication*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->setShouldClose(glfwWindowShouldClose(window));
    }
}

bool NativeApplication::initPlatform() {
    // Initialize GLFW
    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window = glfwCreateWindow(windowWidth, windowHeight, "WebGPU Triangle - Native", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);

    return true;
}

bool NativeApplication::initWebGPU() {
    // Initialize Dawn procedures first
    DawnProcTable procs = dawn::native::GetProcs();
    dawnProcSetProcs(&procs);

    // Create native instance directly for better control
    std::unique_ptr<dawn::native::Instance> nativeInstance = std::make_unique<dawn::native::Instance>();
    if (!nativeInstance) {
        std::cerr << "Failed to create Dawn native instance" << std::endl;
        return false;
    }

    // Get WebGPU instance from native instance
    instance = wgpu::Instance(nativeInstance->Get());
    if (!instance) {
        std::cerr << "Failed to get WebGPU instance from native instance" << std::endl;
        return false;
    }

    // Create surface for the window
    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
    if (!surface) {
        std::cerr << "Failed to create surface" << std::endl;
        return false;
    }

    // Enumerate adapters with the native instance directly
    std::vector<dawn::native::Adapter> adapters = nativeInstance->EnumerateAdapters();
    
    if (adapters.empty()) {
        std::cerr << "No adapters found" << std::endl;
        return false;
    }

    adapter = wgpu::Adapter(adapters[0].Get());

    // Create device
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "Main Device";
    device = adapter.CreateDevice(&deviceDesc);
    if (!device) {
        std::cerr << "Failed to create device" << std::endl;
        return false;
    }

    configureSurface();
    return true;
}

void NativeApplication::handleEvents() {
    glfwPollEvents();
    shouldClose = glfwWindowShouldClose(window);
}

void NativeApplication::cleanup() {
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

} // namespace rs_engine
