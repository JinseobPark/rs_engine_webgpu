#pragma once

#include "../core/Application.h"
#include <emscripten.h>
#include <emscripten/html5.h>

namespace rs_engine {

class WebApplication : public Application {
private:
    const char* canvasId = "#canvas";
    bool adapterReceived = false;
    bool deviceReceived = false;
    bool isCleanedUp = false;  // cleanup 상태 추적

    // Synchronous async wrapper state
    bool initializationComplete = false;
    bool initializationFailed = false;

    static void onDeviceError(WGPUErrorType type, char const* message, void* userdata);
    static void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata);
    static void onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata);
    static EM_BOOL keyCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData);

    void onAdapterReceived();
    void onDeviceReceived();
    bool createSurface();
    bool waitForInitialization();

public:
    bool initPlatform() override;
    bool initWebGPU() override;
    void handleEvents() override;
    void cleanup() override;
};

} // namespace rs_engine
