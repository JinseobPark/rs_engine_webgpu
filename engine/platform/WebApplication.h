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

    static void onDeviceError(WGPUErrorType type, char const* message, void* userdata);
    static void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata);
    static void onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata);
    static void renderLoop(void* userData);
    static EM_BOOL keyCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData);

    void onAdapterReceived();
    void onDeviceReceived();
    bool createSurface();

public:
    bool init() override;
    bool initPlatform() override;
    bool initWebGPU() override;
    void handleEvents() override;
    void cleanup() override;

    // 웹 전용 run 메서드 (부모의 run을 오버라이드하지 않음)
    void run();
};

} // namespace rs_engine
