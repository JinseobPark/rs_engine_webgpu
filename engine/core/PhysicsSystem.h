#pragma once

#include "IEngineSystem.h"
#include "../physics/PhysicsWorld.h"
#include <memory>

namespace rs_engine {

// Forward declaration
class ApplicationSystem;

/**
 * @brief Physics System - Simulation coordinator
 * 
 * Responsibilities:
 * - Physics world management
 * - Fixed timestep updates
 * - Quality adjustment based on performance
 * - Collision detection
 * - Constraint solving
 * 
 * Platform Support: 100% shared
 */
class PhysicsSystem : public IEngineSystem {
private:
    ApplicationSystem* appSystem = nullptr;
    std::unique_ptr<PhysicsWorld> physicsWorld;
    
    float fixedTimeStep = 1.0f / 60.0f; // 60Hz
    float accumulator = 0.0f;
    
    bool enabled = true;

public:
    PhysicsSystem() = default;
    virtual ~PhysicsSystem() = default;

    // IEngineSystem interface
    bool initialize(Engine* engineRef) override;
    void onStart() override;
    void onUpdate(float deltaTime) override;
    void onFixedUpdate(float fixedDeltaTime) override;
    void onShutdown() override;
    
    const char* getName() const override { return "Physics"; }
    int getPriority() const override { return 50; }
    
    void setEnabled(bool value) override { enabled = value; }
    bool isEnabled() const override { return enabled; }

    // Physics world access
    PhysicsWorld* getPhysicsWorld() { return physicsWorld.get(); }
    
    // Configuration
    void setFixedTimeStep(float step) { fixedTimeStep = step; }
    float getFixedTimeStep() const { return fixedTimeStep; }
    
    // Quality control
    void setQuality(float quality);
    float getQuality() const;
    
    // Simulation control
    void setPaused(bool paused);
    bool isPaused() const;
    void setTimeScale(float scale);
    float getTimeScale() const;

private:
    float quality = 1.0f;
    bool paused = false;
    float timeScale = 1.0f;
};

} // namespace rs_engine
