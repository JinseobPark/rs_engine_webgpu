#include "ApplicationSystem.h"
#include "../input/InputSystem.h"
#include "../../core/Engine.h"
#include <iostream>
#include <cassert>

namespace rs_engine {

#ifndef __EMSCRIPTEN__
bool ApplicationSystem::s_glfwInitialized = false;
#endif

bool ApplicationSystem::initialize(Engine* engineRef) {
    if (!IEngineSystem::initialize(engineRef)) {
        return false;
    }

    std::cout << "[INFO] Initializing Application System..." << std::endl;

    if (!initPlatform()) {
        std::cerr << "[ERROR] Failed to initialize platform" << std::endl;
        return false;
    }

    if (!initWebGPU()) {
        std::cerr << "[ERROR] Failed to initialize WebGPU" << std::endl;
        return false;
    }

    configureSurface();

    std::cout << "[SUCCESS] Application System initialized" << std::endl;
    return true;
}

void ApplicationSystem::onStart() {
    std::cout << "[Application] Started - Window: " << windowWidth << "x" << windowHeight << std::endl;
}

void ApplicationSystem::onUpdate(float deltaTime) {
    handleEvents();
}

void ApplicationSystem::onShutdown() {
    std::cout << "[Application] Shutting down..." << std::endl;

#ifndef __EMSCRIPTEN__
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if (s_glfwInitialized) {
        glfwTerminate();
        s_glfwInitialized = false;
    }
#endif

    device = nullptr;
    surface = nullptr;
    adapter = nullptr;
    instance = nullptr;
}

#ifdef __EMSCRIPTEN__
// ========== Web Platform Implementation ==========

bool ApplicationSystem::initPlatform() {
    std::cout << "[INFO] Initializing Web Platform (Emscripten)" << std::endl;
    // Web platform doesn't need window creation - canvas is in HTML
    return true;
}

bool ApplicationSystem::initWebGPU() {
    std::cout << "[INFO] Initializing WebGPU (Browser)" << std::endl;

    // Create WebGPU instance (using default descriptor for browser)
    // Note: In Emscripten, wgpu::CreateInstance() automatically uses navigator.gpu
    instance = wgpu::CreateInstance(nullptr);
    if (!instance) {
        std::cerr << "[ERROR] Failed to create WebGPU instance" << std::endl;
        std::cerr << "   Make sure your browser supports WebGPU" << std::endl;
        return false;
    }
    
    std::cout << "[SUCCESS] WebGPU instance created" << std::endl;

    // Request adapter (async in browser, but we handle it synchronously via Asyncify)
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;

    struct AdapterData {
        wgpu::Adapter adapter;
        bool requestEnded = false;
    } adapterData;

    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, 
                                     const char* message, void* userdata) {
        AdapterData* data = static_cast<AdapterData*>(userdata);
        if (status == WGPURequestAdapterStatus_Success) {
            data->adapter = wgpu::Adapter::Acquire(adapter);
        } else {
            std::cerr << "[ERROR] Adapter request failed: " << message << std::endl;
        }
        data->requestEnded = true;
    };

    instance.RequestAdapter(&adapterOpts, onAdapterRequestEnded, &adapterData);

    // Wait for adapter (Asyncify handles this)
    while (!adapterData.requestEnded) {
        emscripten_sleep(10);
    }

    adapter = adapterData.adapter;
    if (!adapter) {
        return false;
    }

    // Request device
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.defaultQueue.label = "Default Queue";

    struct DeviceData {
        wgpu::Device device;
        bool requestEnded = false;
    } deviceData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device,
                                    const char* message, void* userdata) {
        DeviceData* data = static_cast<DeviceData*>(userdata);
        if (status == WGPURequestDeviceStatus_Success) {
            data->device = wgpu::Device::Acquire(device);
        } else {
            std::cerr << "[ERROR] Device request failed: " << message << std::endl;
        }
        data->requestEnded = true;
    };

    adapter.RequestDevice(&deviceDesc, onDeviceRequestEnded, &deviceData);

    while (!deviceData.requestEnded) {
        emscripten_sleep(10);
    }

    device = deviceData.device;
    if (!device) {
        return false;
    }

    // Set error callback (note: not all browsers support this yet)
    device.SetUncapturedErrorCallback(
        [](WGPUErrorType type, const char* message, void* userdata) {
            const char* errorType = "Unknown";
            switch (type) {
                case WGPUErrorType_Validation: errorType = "Validation"; break;
                case WGPUErrorType_OutOfMemory: errorType = "OutOfMemory"; break;
                case WGPUErrorType_Internal: errorType = "Internal"; break;
                case WGPUErrorType_Unknown: errorType = "Unknown"; break;
                case WGPUErrorType_DeviceLost: errorType = "DeviceLost"; break;
                default: break;
            }
            std::cerr << "WebGPU Error (" << errorType << "): " << message << std::endl;
        }, nullptr);

    // Get surface from canvas
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
    canvasDesc.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = &canvasDesc;

    surface = instance.CreateSurface(&surfaceDesc);
    if (!surface) {
        std::cerr << "[ERROR] Failed to create surface" << std::endl;
        return false;
    }

    std::cout << "[SUCCESS] WebGPU initialized (Web)" << std::endl;
    return true;
}

void ApplicationSystem::handleEvents() {
    // Web events are handled by browser
}

void ApplicationSystem::configureSurface() {
    wgpu::SurfaceConfiguration surfaceConfig = {};
    surfaceConfig.device = device;
    surfaceConfig.format = wgpu::TextureFormat::BGRA8Unorm;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
    surfaceConfig.width = windowWidth;
    surfaceConfig.height = windowHeight;
    surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
    
    surface.Configure(&surfaceConfig);
}

#else
// ========== Native Platform Implementation ==========

bool ApplicationSystem::initPlatform() {
    std::cout << "[INFO] Initializing Native Platform (GLFW)" << std::endl;

    if (!s_glfwInitialized) {
        glfwSetErrorCallback(errorCallback);

        if (!glfwInit()) {
            std::cerr << "[ERROR] Failed to initialize GLFW" << std::endl;
            return false;
        }

        s_glfwInitialized = true;
        std::cout << "[SUCCESS] GLFW initialized" << std::endl;
    }

    // Configure GLFW for WebGPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(windowWidth, windowHeight, "RS Engine WebGPU", nullptr, nullptr);
    if (!window) {
        std::cerr << "[ERROR] Failed to create GLFW window" << std::endl;
        return false;
    }

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(window, this);

    // Set callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    std::cout << "[SUCCESS] Native window created" << std::endl;
    return true;
}

bool ApplicationSystem::initWebGPU() {
    std::cout << "[INFO] Initializing WebGPU (Dawn)" << std::endl;

    // Initialize Dawn procedures first
    DawnProcTable procs = dawn::native::GetProcs();
    dawnProcSetProcs(&procs);

    // Create native instance
    dawnInstance = std::make_unique<dawn::native::Instance>();
    if (!dawnInstance) {
        std::cerr << "[ERROR] Failed to create Dawn native instance" << std::endl;
        return false;
    }

    // Get WebGPU instance from native instance
    instance = wgpu::Instance(dawnInstance->Get());
    if (!instance) {
        std::cerr << "[ERROR] Failed to get WebGPU instance" << std::endl;
        return false;
    }

    // Create surface for the window
    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
    if (!surface) {
        std::cerr << "[ERROR] Failed to create surface" << std::endl;
        return false;
    }

    // Enumerate adapters
    std::vector<dawn::native::Adapter> adapters = dawnInstance->EnumerateAdapters();
    if (adapters.empty()) {
        std::cerr << "[ERROR] No WebGPU adapters found" << std::endl;
        return false;
    }

    adapter = wgpu::Adapter(adapters[0].Get());

    // Create device
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "Main Device";
    device = adapter.CreateDevice(&deviceDesc);
    
    if (!device) {
        std::cerr << "[ERROR] Failed to create device" << std::endl;
        return false;
    }

    std::cout << "[SUCCESS] WebGPU initialized (Native)" << std::endl;
    return true;
}

void ApplicationSystem::handleEvents() {
    glfwPollEvents();
    
    if (glfwWindowShouldClose(window)) {
        shouldCloseFlag = true;
        if (engine) {
            engine->stop();
        }
    }
}

void ApplicationSystem::configureSurface() {
    wgpu::SurfaceConfiguration surfaceConfig = {};
    surfaceConfig.device = device;
    surfaceConfig.format = wgpu::TextureFormat::BGRA8Unorm;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
    surfaceConfig.width = windowWidth;
    surfaceConfig.height = windowHeight;
    surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
    
    surface.Configure(&surfaceConfig);
}

void ApplicationSystem::onWindowResize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    configureSurface();
    std::cout << "[Application] Window resized to " << width << "x" << height << std::endl;
}

// GLFW Callbacks
void ApplicationSystem::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void ApplicationSystem::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* app = static_cast<ApplicationSystem*>(glfwGetWindowUserPointer(window));
    if (!app || !app->engine) return;
    
    // Forward to InputSystem if available
    auto* inputSystem = app->engine->getSystem<InputSystem>();
    if (inputSystem) {
        bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        inputSystem->updateKeyState(key, pressed);
    }
    
    // Handle ESC to close (fallback if InputSystem not present)
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void ApplicationSystem::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* app = static_cast<ApplicationSystem*>(glfwGetWindowUserPointer(window));
    if (!app || !app->engine) return;
    
    auto* inputSystem = app->engine->getSystem<InputSystem>();
    if (inputSystem) {
        bool pressed = (action == GLFW_PRESS);
        inputSystem->updateMouseButtonState(button, pressed);
    }
}

void ApplicationSystem::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* app = static_cast<ApplicationSystem*>(glfwGetWindowUserPointer(window));
    if (!app || !app->engine) return;
    
    auto* inputSystem = app->engine->getSystem<InputSystem>();
    if (inputSystem) {
        inputSystem->updateMousePosition(xpos, ypos);
    }
}

void ApplicationSystem::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* app = static_cast<ApplicationSystem*>(glfwGetWindowUserPointer(window));
    if (!app || !app->engine) return;
    
    auto* inputSystem = app->engine->getSystem<InputSystem>();
    if (inputSystem) {
        inputSystem->updateScroll(xoffset, yoffset);
    }
}

void ApplicationSystem::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<ApplicationSystem*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->onWindowResize(width, height);
    }
}

#endif

} // namespace rs_engine
