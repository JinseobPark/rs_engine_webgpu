#include "WebApplication.h"
#include <emscripten/html5_webgpu.h>

namespace rs_engine {

void WebApplication::onDeviceError(WGPUErrorType type, char const* message, void* userdata) {
    std::cerr << "Device error: " << message << std::endl;
}

void WebApplication::onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata) {
    WebApplication* app = static_cast<WebApplication*>(userdata);
    if (status == WGPURequestAdapterStatus_Success && adapter) {
        app->adapter = wgpu::Adapter::Acquire(adapter);
        app->onAdapterReceived();
    } else {
        std::cerr << "Could not get WebGPU adapter: " << message << std::endl;
        app->initializationFailed = true;
    }
}

void WebApplication::onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
    WebApplication* app = static_cast<WebApplication*>(userdata);
    if (status == WGPURequestDeviceStatus_Success) {
        app->device = wgpu::Device::Acquire(device);
        app->onDeviceReceived();
    } else {
        std::cerr << "Could not get WebGPU device: " << message << std::endl;
        app->initializationFailed = true;
    }
}


EM_BOOL WebApplication::keyCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
    WebApplication* app = static_cast<WebApplication*>(userData);

    if (eventType == EMSCRIPTEN_EVENT_KEYDOWN) {
        if (strcmp(keyEvent->key, "Escape") == 0) {
            std::cout << "🔑 ESC key pressed, closing application..." << std::endl;
            app->shouldClose = true;
            app->cleanup();
        }
    }

    return EM_TRUE;
}

void WebApplication::onAdapterReceived() {
    std::cout << "Adapter received, requesting device..." << std::endl;

    if (!adapter) {
        std::cerr << "Invalid adapter received!" << std::endl;
        return;
    }

    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "Web Device";

    adapter.RequestDevice(&deviceDesc, onDeviceRequestEnded, this);
    adapterReceived = true;
}

void WebApplication::onDeviceReceived() {
    std::cout << "Device received, completing initialization..." << std::endl;

    if (!device) {
        std::cerr << "Invalid device received!" << std::endl;
        initializationFailed = true;
        return;
    }

    // Set error callback
    device.SetUncapturedErrorCallback(onDeviceError, this);

    deviceReceived = true;
    initializationComplete = true;

    std::cout << "WebGPU initialization completed" << std::endl;

    // Initialize renderer after device is ready
    std::cout << "🎯 Initializing scene after WebGPU device ready..." << std::endl;
    if (!initializeScene()) {
        std::cerr << "❌ Failed to initialize scene" << std::endl;
        initializationFailed = true;
        return;
    }

    // // Initialize GUI after scene
    // std::cout << "🎯 Initializing GUI after scene ready..." << std::endl;
    // if (!initializeGUI()) {
    //     std::cerr << "❌ Failed to initialize GUI" << std::endl;
    //     initializationFailed = true;
    //     return;
    // }

    // Surface configuration
    configureSurface();

    std::cout << "✅ Web application fully initialized!" << std::endl;
}

bool WebApplication::createSurface() {
    // For Emscripten/Web, create surface using canvas HTML selector
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector htmlSurfaceDesc = {};
    htmlSurfaceDesc.selector = canvasId;

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = reinterpret_cast<wgpu::ChainedStruct*>(&htmlSurfaceDesc);

        surface = instance.CreateSurface(&surfaceDesc);
    return surface != nullptr;
}

bool WebApplication::initPlatform() {
    // Set up keyboard event handlers
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, keyCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, keyCallback);

    return true;
}

bool WebApplication::initWebGPU() {
    // Create WebGPU instance (emscripten requires nullptr)
    instance = wgpu::CreateInstance(nullptr);
    if (!instance) {
        std::cerr << "Failed to create WebGPU instance" << std::endl;
        return false;
    }

    // Request adapter
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;

    instance.RequestAdapter(&adapterOpts, onAdapterRequestEnded, this);

    // Wait for async initialization to complete
    if (!waitForInitialization()) {
        return false;
    }

    // Create surface now that we have the device
    if (!createSurface()) {
        std::cerr << "Failed to create surface" << std::endl;
        return false;
    }

    configureSurface();
    return true;
}

void WebApplication::handleEvents() {
    // Web에서는 이벤트가 자동으로 처리됨 (emscripten callback)
}

void WebApplication::cleanup() {
    // 이미 cleanup이 호출된 경우 중복 실행 방지
    if (isCleanedUp) {
        return;
    }

    std::cout << "🧹 Cleaning up WebApplication..." << std::endl;

    // 메인 루프 정지
    emscripten_cancel_main_loop();

    // 키보드 이벤트 콜백 해제
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, nullptr);

    // WebGPU 리소스 해제 (순서 중요)
    // if (guiManager) {
    //     guiManager.reset();
    // }

    if (scene) {
        scene.reset();
    }

    if (surface) {
        surface = nullptr;
    }

    if (device) {
        device = nullptr;
    }

    if (adapter) {
        adapter = nullptr;
    }

    if (instance) {
        instance = nullptr;
    }

    isCleanedUp = true;
    std::cout << "✅ WebApplication cleanup completed" << std::endl;
}

bool WebApplication::waitForInitialization() {
    std::cout << "🔄 Waiting for WebGPU initialization..." << std::endl;

    // Reset state
    initializationComplete = false;
    initializationFailed = false;

    // Wait for async operations to complete using emscripten_sleep
    const int maxWaitMs = 5000; // 5 second timeout
    const int sleepMs = 10;
    int totalWaitMs = 0;

    while (!initializationComplete && !initializationFailed && totalWaitMs < maxWaitMs) {
        emscripten_sleep(sleepMs);
        totalWaitMs += sleepMs;

        // Log progress every second
        if (totalWaitMs % 1000 == 0) {
            std::cout << "⏰ Waiting... " << (totalWaitMs / 1000) << "s elapsed" << std::endl;
        }
    }

    if (initializationFailed) {
        std::cerr << "❌ WebGPU initialization failed after " << totalWaitMs << "ms" << std::endl;
        return false;
    }

    if (!initializationComplete) {
        std::cerr << "⏰ WebGPU initialization timed out after " << totalWaitMs << "ms" << std::endl;
        return false;
    }

    std::cout << "✅ WebGPU initialization completed in " << totalWaitMs << "ms" << std::endl;
    return true;
}

} // namespace rs_engine