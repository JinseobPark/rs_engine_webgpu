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

bool Engine::createSceneObject(const std::string& name) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (!renderSystem || !renderSystem->getScene()) {
        return false;
    }
    
    return renderSystem->getScene()->createObject(name) != nullptr;
}

bool Engine::addMeshToSceneObject(const std::string& objectName, uint64_t meshHandle) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (!renderSystem || !renderSystem->getScene()) {
        return false;
    }
    
    return renderSystem->getScene()->addMeshToObject(objectName, meshHandle);
}

void Engine::setObjectPosition(const std::string& name, const Vec3& position) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        auto* object = renderSystem->getScene()->getObject(name);
        if (object) {
            object->setPosition(position);
        }
    }
}

void Engine::setObjectScale(const std::string& name, const Vec3& scale) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        auto* object = renderSystem->getScene()->getObject(name);
        if (object) {
            object->setScale(scale);
        }
    }
}

void Engine::removeSceneObject(const std::string& name) {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        renderSystem->getScene()->removeObject(name);
    }
}

void Engine::clearScene() {
    auto* renderSystem = getSystem<RenderSystem>();
    if (renderSystem && renderSystem->getScene()) {
        renderSystem->getScene()->clearAllObjects();
    }
}

// ========== Input Control ==========

bool Engine::isKeyPressed(int keyCode) const {
    auto* inputSystem = const_cast<Engine*>(this)->getSystem<InputSystem>();
    return inputSystem ? inputSystem->isKeyPressed(static_cast<KeyCode>(keyCode)) : false;
}

bool Engine::isKeyHeld(int keyCode) const {
    auto* inputSystem = const_cast<Engine*>(this)->getSystem<InputSystem>();
    return inputSystem ? inputSystem->isKeyHeld(static_cast<KeyCode>(keyCode)) : false;
}

bool Engine::isKeyDown(int keyCode) const {
    auto* inputSystem = const_cast<Engine*>(this)->getSystem<InputSystem>();
    return inputSystem ? inputSystem->isKeyDown(static_cast<KeyCode>(keyCode)) : false;
}

bool Engine::isMouseButtonPressed(int button) const {
    auto* inputSystem = const_cast<Engine*>(this)->getSystem<InputSystem>();
    return inputSystem ? inputSystem->isMouseButtonPressed(static_cast<MouseButton>(button)) : false;
}

bool Engine::isMouseButtonDown(int button) const {
    auto* inputSystem = const_cast<Engine*>(this)->getSystem<InputSystem>();
    return inputSystem ? inputSystem->isMouseButtonDown(static_cast<MouseButton>(button)) : false;
}

void Engine::getMousePosition(double& x, double& y) const {
    auto* inputSystem = const_cast<Engine*>(this)->getSystem<InputSystem>();
    if (inputSystem) {
        inputSystem->getMousePosition(x, y);
    } else {
        x = 0.0;
        y = 0.0;
    }
}

void Engine::getMouseDelta(double& dx, double& dy) const {
    auto* inputSystem = const_cast<Engine*>(this)->getSystem<InputSystem>();
    if (inputSystem) {
        inputSystem->getMouseDelta(dx, dy);
    } else {
        dx = 0.0;
        dy = 0.0;
    }
}

void Engine::lockCursor(bool lock) {
    auto* inputSystem = getSystem<InputSystem>();
    if (inputSystem) {
        inputSystem->lockCursor(lock);
    }
}

void Engine::showCursor(bool show) {
    auto* inputSystem = getSystem<InputSystem>();
    if (inputSystem) {
        inputSystem->showCursor(show);
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

// ========== Resource Control ==========

uint64_t Engine::createCubeMesh(const std::string& name, float size) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        return resourceSystem->createCubeMesh(name, size);
    }
    return 0; // INVALID_RESOURCE_HANDLE
}

uint64_t Engine::createSphereMesh(const std::string& name, float radius, int segments) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        return resourceSystem->createSphereMesh(name, radius, segments);
    }
    return 0;
}

uint64_t Engine::createPlaneMesh(const std::string& name, float width, float height) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        return resourceSystem->createPlaneMesh(name, width, height);
    }
    return 0;
}

uint64_t Engine::createSolidColorTexture(const std::string& name,
                                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        return resourceSystem->createSolidColorTexture(name, r, g, b, a);
    }
    return 0;
}

uint64_t Engine::createCheckerboardTexture(const std::string& name,
                                           uint32_t size, uint32_t checkSize) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        return resourceSystem->createCheckerboardTexture(name, size, checkSize);
    }
    return 0;
}

uint64_t Engine::loadModel(const std::string& name, const std::string& filepath) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        return resourceSystem->loadModel(name, filepath);
    }
    return 0;
}

uint64_t Engine::loadTexture(const std::string& name, const std::string& filepath) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        return resourceSystem->loadTexture(name, filepath);
    }
    return 0;
}

void Engine::removeResource(const std::string& name) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        resourceSystem->removeResource(name);
    }
}

void Engine::removeResource(uint64_t handle) {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        resourceSystem->removeResource(handle);
    }
}

void Engine::clearAllResources() {
    auto* resourceSystem = getSystem<ResourceSystem>();
    if (resourceSystem) {
        resourceSystem->clearAllResources();
    }
}

bool Engine::hasResource(const std::string& name) const {
    auto* resourceSystem = const_cast<Engine*>(this)->getSystem<ResourceSystem>();
    return resourceSystem ? resourceSystem->hasResource(name) : false;
}

bool Engine::hasResource(uint64_t handle) const {
    auto* resourceSystem = const_cast<Engine*>(this)->getSystem<ResourceSystem>();
    return resourceSystem ? resourceSystem->hasResource(handle) : false;
}

size_t Engine::getResourceCount() const {
    auto* resourceSystem = const_cast<Engine*>(this)->getSystem<ResourceSystem>();
    return resourceSystem ? resourceSystem->getResourceCount() : 0;
}

size_t Engine::getResourceMemoryUsed() const {
    auto* resourceSystem = const_cast<Engine*>(this)->getSystem<ResourceSystem>();
    return resourceSystem ? resourceSystem->getTotalMemoryUsed() : 0;
}

size_t Engine::getResourceGPUMemoryUsed() const {
    auto* resourceSystem = const_cast<Engine*>(this)->getSystem<ResourceSystem>();
    return resourceSystem ? resourceSystem->getGPUMemoryUsed() : 0;
}

void Engine::printResourceStatistics() const {
    auto* resourceSystem = const_cast<Engine*>(this)->getSystem<ResourceSystem>();
    if (resourceSystem) {
        resourceSystem->printStatistics();
    }
}

} // namespace rs_engine
