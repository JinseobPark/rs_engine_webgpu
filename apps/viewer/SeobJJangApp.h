#pragma once

#include "engine/core/Engine.h"
#include <iostream>

// Forward declarations
namespace rs_engine {
    class RenderSystem;
    class ResourceSystem;
    class PhysicsSystem;
    class InputSystem;
}

/**
 * @brief SeobJJang Viewer Application
 * 
 * Demonstrates REFACTORED architecture with direct system access:
 * - App knows Engine for lifecycle management
 * - App accesses systems directly via getSystem<T>()
 * - NO unnecessary Engine forwarding methods
 * - More explicit, cleaner code
 * 
 * This app shows:
 * - Automatic system initialization by Engine
 * - Direct system access for scene setup
 * - Direct camera control via RenderSystem
 * - Direct physics control via PhysicsSystem
 * - Main loop using Engine::shouldClose()
 */
class SeobJJangApp {
public: // Make systems public for JS API access
    rs_engine::Engine engine;

    // Cached system references (initialized in init())
    rs_engine::RenderSystem* renderSystem = nullptr;
    rs_engine::ResourceSystem* resourceSystem = nullptr;
    rs_engine::PhysicsSystem* physicsSystem = nullptr;

public:
    SeobJJangApp();
    ~SeobJJangApp();

    bool init();
    void run();
    void shutdown();

private:
    void setupScene();
};
