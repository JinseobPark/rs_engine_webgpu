#pragma once

#include "engine/core/Engine.h"
#include "engine/physics/PhysicsWorld.h"
#include "engine/physics/fluid/SPHSimulation.h"
#include <memory>
#include <iostream>

class FluidDemoApp {
private:
    rs_engine::Engine engine;
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