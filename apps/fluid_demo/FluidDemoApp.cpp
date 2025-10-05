#include "FluidDemoApp.h"
#include "engine/core/Config.h"
#include "engine/systems/rendering/RenderSystem.h"
#include "engine/systems/physics/PhysicsSystem.h"
#include "engine/rendering/scene/Scene.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using rs_engine::RenderSystem;
using rs_engine::PhysicsSystem;

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

    // Cache system references
    renderSystem = engine.getSystem<RenderSystem>();
    physicsSystem = engine.getSystem<PhysicsSystem>();
    
    if (!renderSystem || !physicsSystem) {
        std::cerr << "[ERROR] Required systems not found!" << std::endl;
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
    // Setup camera for fluid viewing (direct access)
    auto* scene = renderSystem->getScene();
    if (scene) {
        auto* camera = scene->getCamera();
        if (camera) {
            camera->setPosition(rs_engine::Vec3(0.0f, 5.0f, 10.0f));
            camera->setTarget(rs_engine::Vec3(0.0f, 0.0f, 0.0f));
            camera->setFOV(60.0f);
            std::cout << "   [INFO] Camera positioned for fluid demo" << std::endl;
        }
    }
    
    // Set physics quality (direct access)
    physicsSystem->setQuality(1.0f);
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