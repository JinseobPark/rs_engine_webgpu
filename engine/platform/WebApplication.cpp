#include "WebApplication.h"
#include <emscripten/html5_webgpu.h>

namespace rs_engine {

void WebApplication::onDeviceError(WGPUErrorType type, char const* message, void* userdata) {
    std::cerr << "Device error: " << message << std::endl;
}

void WebApplication::onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata) {
    if (status == WGPURequestAdapterStatus_Success && adapter) {
        WebApplication* app = static_cast<WebApplication*>(userdata);
        app->adapter = wgpu::Adapter::Acquire(adapter);
        app->onAdapterReceived();
    } else {
        std::cerr << "Could not get WebGPU adapter: " << message << std::endl;
    }
}

void WebApplication::onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
    if (status == WGPURequestDeviceStatus_Success) {
        WebApplication* app = static_cast<WebApplication*>(userdata);
        app->device = wgpu::Device::Acquire(device);
        app->onDeviceReceived();
    } else {
        std::cerr << "Could not get WebGPU device: " << message << std::endl;
    }
}

void WebApplication::renderLoop(void* userData) {
    WebApplication* app = static_cast<WebApplication*>(userData);
    if (app->isInitialized && !app->shouldClose) {
        app->update(0.016f); // 60fps
        app->draw();
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
    std::cout << "Device received, setting up rendering..." << std::endl;

    if (!device) {
        std::cerr << "Invalid device received!" << std::endl;
        return;
    }

    // Set error callback
    device.SetUncapturedErrorCallback(onDeviceError, this);

    if (!createSurface()) {
        std::cerr << "Failed to create surface" << std::endl;
        return;
    }

    configureSurface();

    if (!createRenderPipeline()) {
        std::cerr << "Failed to create render pipeline" << std::endl;
        return;
    }

    if (!onInit()) {
        std::cerr << "Application initialization failed" << std::endl;
        return;
    }

    deviceReceived = true;
    isInitialized = true;

    // Start the render loop
    emscripten_set_main_loop_arg(renderLoop, this, 60, 1);

    std::cout << "🎯 Triangle App initialized successfully!" << std::endl;
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
    if (pipeline) {
        pipeline = nullptr;
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

bool WebApplication::init() {
    // 웹에서는 비동기 초기화
    return initPlatform() && initWebGPU();
}

void WebApplication::run() {
    // 웹에서는 비동기적으로 초기화되므로 여기서는 대기만 함
    // 실제 렌더 루프는 onDeviceReceived에서 시작됨
}

} // namespace rs_engine