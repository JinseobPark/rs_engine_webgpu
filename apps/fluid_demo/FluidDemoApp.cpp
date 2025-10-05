#include "FluidDemoApp.h"
#include "engine/core/Config.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

FluidDemoApp::FluidDemoApp() {
    std::cout << "[INFO] Creating Fluid Demo App..." << std::endl;
}

FluidDemoApp::~FluidDemoApp() {
    shutdown();
}

bool FluidDemoApp::init() {
    // Initialize engine (systems added automatically)
    if (!engine.initialize()) {
        std::cerr << "[ERROR] Failed to initialize engine" << std::endl;
        return false;
    }

    // Start engine
    engine.start();

    std::cout << "[SUCCESS] Fluid Demo initialized!" << std::endl;
    
    // Platform limits
    auto limits = rs_engine::EngineConfig::getLimits();
    std::cout << "Platform limits: " << std::endl;
    std::cout << "  Max particles: " << limits.maxParticles << std::endl;
    std::cout << "  Advanced features: " << (limits.enableAdvancedFeatures ? "ON" : "OFF") << std::endl;

    // Setup scene through Engine interface
    setupScene();
    
    std::cout << "Scene setup complete\n" << std::endl;
    return true;
}

void FluidDemoApp::setupScene() {
    // Setup camera for fluid viewing
    engine.setCameraPosition(rs_engine::Vec3(0.0f, 5.0f, 10.0f));
    engine.setCameraTarget(rs_engine::Vec3(0.0f, 0.0f, 0.0f));
    engine.setCameraFOV(60.0f);
    
    // Set physics quality
    engine.setPhysicsQuality(1.0f);
    
    std::cout << "   [INFO] Camera positioned for fluid demo" << std::endl;
    std::cout << "   [INFO] Physics quality set to 1.0" << std::endl;
}

void FluidDemoApp::run() {
#ifdef __EMSCRIPTEN__
    // Web: Set up main loop callback
    emscripten_set_main_loop_arg(
        [](void* userData) {
            FluidDemoApp* app = static_cast<FluidDemoApp*>(userData);
            app->engine.update();
        },
        this, 0, 1);
#else
    // Native: Direct loop
    std::cout << "[INFO] Starting Fluid Demo main loop...\n" << std::endl;
    while (!engine.shouldClose()) {
        engine.update();
    }
    std::cout << "\n[INFO] Fluid Demo ended" << std::endl;
#endif
}

void FluidDemoApp::shutdown() {
    std::cout << "[INFO] Cleaning up Fluid Demo..." << std::endl;
    engine.shutdown();
    std::cout << "[SUCCESS] Cleanup complete" << std::endl;
}