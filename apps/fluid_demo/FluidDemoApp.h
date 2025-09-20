#pragma once

#ifdef __EMSCRIPTEN__
    #include "engine/platform/WebApplication.h"
    using BaseApp = rs_engine::WebApplication;
#else
    #include "engine/platform/NativeApplication.h"
    using BaseApp = rs_engine::NativeApplication;
#endif

#include "engine/physics/PhysicsWorld.h"
#include "engine/physics/fluid/SPHSimulation.h"

class FluidDemoApp : public BaseApp {
private:
    std::unique_ptr<rs_engine::PhysicsWorld> physicsWorld;
    std::unique_ptr<rs_engine::SPHSimulation> fluidSim;

public:
    bool onInit() override {
        std::cout << "🌊 Fluid Demo initialized!" << std::endl;
        std::cout << "Platform limits: " << std::endl;
        std::cout << "  Max particles: " << platformLimits.maxParticles << std::endl;
        std::cout << "  Advanced features: " << (platformLimits.enableAdvancedFeatures ? "ON" : "OFF") << std::endl;

        // Initialize physics world
        physicsWorld = std::make_unique<rs_engine::PhysicsWorld>(&device);

        return true;
    }

    void update(float deltaTime) override {
        if (physicsWorld) {
            physicsWorld->update(deltaTime);
            physicsWorld->adjustQualityForPerformance(deltaTime);
        }
        BaseApp::update(deltaTime);
    }

    void draw() override {
        BaseApp::draw();
    }
};