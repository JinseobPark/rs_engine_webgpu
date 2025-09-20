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
    FluidDemoApp();
    ~FluidDemoApp();

    bool onInit() override;

    void update(float deltaTime) override;

    void draw() override;
};