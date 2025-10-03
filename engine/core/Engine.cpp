#include "Engine.h"
#include "ApplicationSystem.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
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
        std::cerr << "⚠️  Engine already initialized" << std::endl;
        return true;
    }

    std::cout << "🚀 Initializing Engine..." << std::endl;
    std::cout << "📦 Platform: "
#ifdef __EMSCRIPTEN__
              << "Web (Emscripten)"
#else
              << "Native (Dawn)"
#endif
              << std::endl;

    // Add default systems if not already added
    if (systems.empty()) {
        std::cout << "🔧 Adding default engine systems..." << std::endl;
        addSystem<ApplicationSystem>();
        addSystem<PhysicsSystem>();
        addSystem<RenderSystem>();
        std::cout << "✅ Default systems added" << std::endl;
    }

    // Sort systems by priority before initialization
    sortSystems();

    // Initialize all systems in priority order
    for (auto& system : systems) {
        std::cout << "   🔧 Initializing " << system->getName() 
                  << " (priority: " << system->getPriority() << ")..." << std::endl;
        
        if (!system->initialize(this)) {
            std::cerr << "❌ Failed to initialize " << system->getName() << std::endl;
            return false;
        }
        
        system->initialized = true;
        std::cout << "   ✅ " << system->getName() << " initialized" << std::endl;
    }

    // Rebuild cache after initialization
    systemsCache.clear();
    for (auto& system : systems) {
        systemsCache.push_back(system.get());
    }

    isInitialized = true;
    std::cout << "✅ Engine initialized with " << systems.size() << " systems" << std::endl;
    return true;
}

void Engine::start() {
    if (!isInitialized) {
        std::cerr << "❌ Cannot start engine - not initialized" << std::endl;
        return;
    }

    std::cout << "🎬 Starting Engine..." << std::endl;

    // Call onStart on all systems
    for (auto* system : systemsCache) {
        system->onStart();
    }

    isRunning = true;
    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;
    
    std::cout << "✅ Engine started" << std::endl;
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

    std::cout << "🛑 Shutting down Engine..." << std::endl;

    isRunning = false;

    // Shutdown systems in reverse order
    for (auto it = systems.rbegin(); it != systems.rend(); ++it) {
        std::cout << "   🔌 Shutting down " << (*it)->getName() << "..." << std::endl;
        (*it)->onShutdown();
    }

    systems.clear();
    systemsCache.clear();
    isInitialized = false;

    std::cout << "✅ Engine shutdown complete" << std::endl;
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

void Engine::getWindowSize(uint32_t& width, uint32_t& height) const {
    auto* appSystem = const_cast<Engine*>(this)->getSystem<ApplicationSystem>();
    if (appSystem) {
        width = appSystem->getWindowWidth();
        height = appSystem->getWindowHeight();
    } else {
        width = 800;
        height = 600;
    }
}

// ========== Scene Control ==========

void Engine::addSceneObject(const std::string& name, 
                           const Vec3& position,
                           const Vec3& scale) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        // Scene의 addCube는 position, rotation, scale을 받음 (name 파라미터 없음)
        renderSystem->getScene()->addCube(position, Vec3(0.0f, 0.0f, 0.0f), scale);
    }
}

void Engine::removeSceneObject(const std::string& name) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        // 향후 Scene에 removeObject 메서드 추가 필요
        std::cerr << "⚠️  removeSceneObject not yet implemented" << std::endl;
    }
}

void Engine::clearScene() {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        // 향후 Scene에 clear 메서드 추가 필요
        std::cerr << "⚠️  clearScene not yet implemented" << std::endl;
    }
}

// ========== Camera Control ==========

void Engine::setCameraPosition(const Vec3& position) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        auto* camera = renderSystem->getScene()->getCamera();
        if (camera) {
            camera->setPosition(position);
        }
    }
}

void Engine::setCameraTarget(const Vec3& target) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        auto* camera = renderSystem->getScene()->getCamera();
        if (camera) {
            camera->setTarget(target);
        }
    }
}

void Engine::setCameraFOV(float fov) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        auto* camera = renderSystem->getScene()->getCamera();
        if (camera) {
            camera->setFOV(fov);
        }
    }
}

// ========== Physics Control ==========

void Engine::setPhysicsQuality(float quality) {
    auto* physicsSystem = getSystem<PhysicsSystem>();
    if (physicsSystem) {
        physicsSystem->setQuality(quality);
    }
}

float Engine::getPhysicsQuality() const {
    auto* physicsSystem = const_cast<Engine*>(this)->getSystem<PhysicsSystem>();
    return physicsSystem ? physicsSystem->getQuality() : 1.0f;
}

void Engine::setPhysicsPaused(bool paused) {
    auto* physicsSystem = getSystem<PhysicsSystem>();
    if (physicsSystem) {
        physicsSystem->setPaused(paused);
    }
}

void Engine::setPhysicsTimeScale(float scale) {
    auto* physicsSystem = getSystem<PhysicsSystem>();
    if (physicsSystem) {
        physicsSystem->setTimeScale(scale);
    }
}

} // namespace rs_engine
