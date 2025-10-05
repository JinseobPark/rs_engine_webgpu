#pragma once

#include "engine/core/Engine.h"
#include "engine/physics/PhysicsWorld.h"
#include "engine/physics/fluid/SPHSimulation.h"
#include <memory>
#include <iostream>

// Forward declarations
namespace rs_engine {
    class RenderSystem;
    class PhysicsSystem;
}

class FluidDemoApp {
private:
    rs_engine::Engine engine;
    
    // Cached system references
    rs_engine::RenderSystem* renderSystem = nullptr;
    rs_engine::PhysicsSystem* physicsSystem = nullptr;
    
    std::unique_ptr<rs_engine::PhysicsWorld> physicsWorld;
    std::unique_ptr<rs_engine::SPHSimulation> fluidSim;

public:
    FluidDemoApp();
    ~FluidDemoApp();

    bool init();
    void run();
    void shutdown();
    
private:
    void setupScene();
};