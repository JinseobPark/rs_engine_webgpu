#include "Engine.h"
#include "../systems/application/ApplicationSystem.h"
#include "../systems/input/InputSystem.h"
#include "../systems/rendering/RenderSystem.h"
#include "../systems/physics/PhysicsSystem.h"
#include "../systems/resource/ResourceSystem.h"
#include <iostream>

namespace rs_engine {

Engine::Engine() {
    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    if (isInitialized) {
        std::cerr << "[WARNING] Engine already initialized" << std::endl;
        return true;
    }

    std::cout << "[INFO] Initializing Engine..." << std::endl;
    std::cout << "[INFO] Platform: "
#ifdef __EMSCRIPTEN__
              << "Web (Emscripten)"
#else
              << "Native (Dawn)"
#endif
              << std::endl;

    // Add default systems if not already added
    if (systems.empty()) {
        std::cout << "[INFO] Adding default engine systems..." << std::endl;
        addSystem<ApplicationSystem>();  // -100: Window, WebGPU, Events
        addSystem<ResourceSystem>();     // -75:  Resources (before Render)
        addSystem<InputSystem>();        // -50:  Input handling
        addSystem<PhysicsSystem>();      // 50:   Physics simulation
        addSystem<RenderSystem>();       // 100:  Rendering
        std::cout << "[SUCCESS] Default systems added" << std::endl;
    }

    // Sort systems by priority before initialization
    sortSystems();

    // Initialize all systems in priority order
    for (auto& system : systems) {
        std::cout << "   [INFO] Initializing " << system->getName() 
                  << " (priority: " << system->getPriority() << ")..." << std::endl;
        
        if (!system->initialize(this)) {
            std::cerr << "[ERROR] Failed to initialize " << system->getName() << std::endl;
            return false;
        }
        
        system->initialized = true;
        std::cout << "   [SUCCESS] " << system->getName() << " initialized" << std::endl;
    }

    // Rebuild cache after initialization
    systemsCache.clear();
    for (auto& system : systems) {
        systemsCache.push_back(system.get());
    }

    isInitialized = true;
    std::cout << "[SUCCESS] Engine initialized with " << systems.size() << " systems" << std::endl;
    return true;
}

void Engine::start() {
    if (!isInitialized) {
        std::cerr << "[ERROR] Cannot start engine - not initialized" << std::endl;
        return;
    }

    std::cout << "[INFO] Starting Engine..." << std::endl;

    // Call onStart on all systems
    for (auto* system : systemsCache) {
        system->onStart();
    }

    isRunning = true;
    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;

    std::cout << "[SUCCESS] Engine started" << std::endl;
}

void Engine::update() {
    if (!isRunning) {
        return;
    }

    // Update time
    updateTime();

    // Update all systems (variable timestep)
    for (auto* system : systemsCache) {
        if (system->isEnabled()) {
            system->onUpdate(deltaTime);
        }
    }

    // Update fixed timestep systems (e.g., physics)
    updateFixedTimestep();
}

void Engine::shutdown() {
    if (!isInitialized) {
        return;
    }

    std::cout << "[INFO] Shutting down Engine..." << std::endl;

    isRunning = false;

    // Shutdown systems in reverse order
    for (auto it = systems.rbegin(); it != systems.rend(); ++it) {
        std::cout << "   [INFO] Shutting down " << (*it)->getName() << "..." << std::endl;
        (*it)->onShutdown();
    }

    systems.clear();
    systemsCache.clear();
    isInitialized = false;

    std::cout << "[SUCCESS] Engine shutdown complete" << std::endl;
}

void Engine::sortSystems() {
    std::sort(systems.begin(), systems.end(),
              [](const std::unique_ptr<IEngineSystem>& a, 
                 const std::unique_ptr<IEngineSystem>& b) {
                  return a->getPriority() < b->getPriority();
              });

    // Rebuild cache
    systemsCache.clear();
    for (auto& system : systems) {
        systemsCache.push_back(system.get());
    }
}

void Engine::updateTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    // Calculate delta time
    std::chrono::duration<float> elapsed = currentTime - lastFrameTime;
    deltaTime = elapsed.count();
    
    // Calculate total time
    std::chrono::duration<double> totalElapsed = currentTime - startTime;
    totalTime = totalElapsed.count();
    
    lastFrameTime = currentTime;
    
    // Clamp delta time to prevent spiral of death
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }
}

void Engine::updateFixedTimestep() {
    fixedAccumulator += deltaTime;

    // Process fixed updates
    while (fixedAccumulator >= fixedTimeStep) {
        for (auto* system : systemsCache) {
            if (system->isEnabled()) {
                system->onFixedUpdate(fixedTimeStep);
            }
        }
        fixedAccumulator -= fixedTimeStep;
    }
}

// ========== Application Control ==========

bool Engine::shouldClose() const {
    auto* appSystem = const_cast<Engine*>(this)->getSystem<ApplicationSystem>();
    return appSystem ? appSystem->shouldClose() : true;
}

// ========== Removed Forwarding Method Implementations ==========
// All scene, camera, input, physics, and resource forwarding methods removed.
// Apps should use getSystem<T>() for direct system access.

} // namespace rs_engine
