#include "FluidDemoApp.h"

FluidDemoApp::FluidDemoApp() {}

FluidDemoApp::~FluidDemoApp() {}

bool FluidDemoApp::onInit()
{
    BaseApp::onInit();
    std::cout << "🌊 Fluid Demo initialized!" << std::endl;
    
    // 브레이크포인트 테스트를 위한 디버거 호출
    #ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log("🛑 FluidDemoApp onInit reached - debugger should break here");
        debugger; // JavaScript 레벨에서 브레이크
    });
    #endif
    
    std::cout << "Platform limits: " << std::endl;
    std::cout << "  Max particles: " << platformLimits.maxParticles << std::endl;
    std::cout << "  Advanced features: " << (platformLimits.enableAdvancedFeatures ? "ON" : "OFF") << std::endl;

    // Initialize physics world
    physicsWorld = std::make_unique<rs_engine::PhysicsWorld>(&device);

    return true;
}

void FluidDemoApp::update(const float deltaTime)
{
    if (physicsWorld) {
            physicsWorld->update(deltaTime);
            physicsWorld->adjustQualityForPerformance(deltaTime);
        }
    BaseApp::update(deltaTime);
}

void FluidDemoApp::draw()
{
    BaseApp::draw();
}