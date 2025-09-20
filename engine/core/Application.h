#pragma once

#include <memory>
#include <iostream>
#include "Config.h"
#include "../rendering/CubeRenderer.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #include <webgpu/webgpu.h>
    #include <webgpu/webgpu_cpp.h>
    #include <emscripten/html5_webgpu.h>
#else
    #define GLFW_NO_API
    #include <GLFW/glfw3.h>
    #include <dawn/dawn_proc.h>
    #include <dawn/native/DawnNative.h>
    #include <dawn/webgpu_cpp.h>
    #include <webgpu/webgpu_glfw.h>
    #include <vector>
#endif

namespace rs_engine {

class Application {
protected:
    // 공통 WebGPU 리소스
    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Surface surface = nullptr;

    // 렌더러
    std::unique_ptr<CubeRenderer> cubeRenderer;

    // 공통 상태
    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
    bool shouldClose = false;
    bool isInitialized = false;

    // 플랫폼별 설정
    PlatformLimits platformLimits = EngineConfig::getLimits();

public:
    virtual ~Application() = default;

    // 플랫폼별 구현 필요
    virtual bool initPlatform() = 0;
    virtual void handleEvents() = 0;
    virtual void cleanup() = 0;

    // 공통 WebGPU 초기화 (플랫폼별로 다름)
    virtual bool initWebGPU() = 0;

    // 공통 구현
    bool initializeRenderer();
    void configureSurface();
    void render();

    // 애플리케이션 로직 (사용자 구현)
    virtual bool init() {
#ifdef __EMSCRIPTEN__
        // Web version: renderer initialization happens after async WebGPU setup
        return initPlatform() && initWebGPU() && onInit();
#else
        // Native version: synchronous initialization
        return initPlatform() && initWebGPU() && initializeRenderer() && onInit();
#endif
    }
    virtual bool onInit() { return true; }
    virtual void update(float deltaTime) {}
    virtual void draw() { render(); }

    // 메인 루프
    void run();

    // 상태 확인
    bool getShouldClose() const { return shouldClose; }
    void setShouldClose(bool value) { shouldClose = value; }
};

} // namespace rs_engine
