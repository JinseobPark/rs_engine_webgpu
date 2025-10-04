#include "PhysicsSystem.h"
#include "../../core/Engine.h"
#include "../application/ApplicationSystem.h"
#include <iostream>

namespace rs_engine {

bool PhysicsSystem::initialize(Engine* engineRef) {
    if (!IEngineSystem::initialize(engineRef)) {
        return false;
    }

    std::cout << "🎯 Initializing Physics System..." << std::endl;

    // Get ApplicationSystem
    appSystem = engine->getSystem<ApplicationSystem>();
    if (!appSystem) {
        std::cerr << "❌ ApplicationSystem not found" << std::endl;
        return false;
    }

    // Create physics world
    physicsWorld = std::make_unique<PhysicsWorld>(&appSystem->getDevice());

    std::cout << "✅ Physics System initialized (Fixed timestep: " 
              << fixedTimeStep << "s)" << std::endl;
    return true;
}

void PhysicsSystem::onStart() {
    std::cout << "[Physics] Started - Quality: " 
              << physicsWorld->getCurrentQuality() << std::endl;
}

void PhysicsSystem::onUpdate(float deltaTime) {
    if (!enabled || !physicsWorld) {
        return;
    }

    // Variable timestep update (for visual effects, interpolation, etc.)
    physicsWorld->update(deltaTime);
}

void PhysicsSystem::onFixedUpdate(float fixedDeltaTime) {
    if (!enabled || !physicsWorld) {
        return;
    }

    // Fixed timestep physics simulation
    // This is called by Engine at a constant rate (60Hz by default)
    // Perfect for deterministic physics simulation
    
    // TODO: Implement actual physics stepping when SPH/PBD systems are ready
}

void PhysicsSystem::onShutdown() {
    std::cout << "[Physics] Shutting down..." << std::endl;
    physicsWorld.reset();
}

// ========== Quality Control ==========

void PhysicsSystem::setQuality(float q) {
    quality = q;
    if (physicsWorld) {
        physicsWorld->setQuality(quality);
    }
}

float PhysicsSystem::getQuality() const {
    return quality;
}

// ========== Simulation Control ==========

void PhysicsSystem::setPaused(bool p) {
    paused = p;
}

bool PhysicsSystem::isPaused() const {
    return paused;
}

void PhysicsSystem::setTimeScale(float scale) {
    timeScale = scale;
}

float PhysicsSystem::getTimeScale() const {
    return timeScale;
}

} // namespace rs_engine
