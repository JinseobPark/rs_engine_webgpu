#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>
#include <cstdint>
#include "IEngineSystem.h"
#include "Config.h"
#include "../core/math/Vec3.h"

namespace rs_engine {

// Forward declarations
class ResourceSystem;

/**
 * @brief Main engine coordinator - manages all subsystems
 * 
 * The Engine class is responsible for:
 * - System lifecycle management (init, update, shutdown)
 * - Frame timing and delta time calculation
 * - Fixed timestep updates for physics
 * - System priority ordering
 * 
 * Platform Support: 100% shared between Web and Native
 * Platform differences are handled by individual systems
 */
class Engine {
private:
    // System storage (owned by engine)
    std::vector<std::unique_ptr<IEngineSystem>> systems;
    
    // System cache (non-owning pointers for fast access)
    std::vector<IEngineSystem*> systemsCache;
    
    // Engine state
    bool isRunning = false;
    bool isInitialized = false;
    
    // Time management
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    double totalTime = 0.0;
    float deltaTime = 0.016f; // Default to 60fps
    
    // Fixed timestep for physics
    float fixedTimeStep = 1.0f / 60.0f; // 60Hz physics
    float fixedAccumulator = 0.0f;

public:
    Engine();
    ~Engine();

    // ========== Lifecycle ==========
    
    /**
     * @brief Initialize all systems in priority order
     * 
     * If no systems have been added, automatically adds default systems:
     * - ApplicationSystem (window, WebGPU, events)
     * - PhysicsSystem (physics simulation)
     * - RenderSystem (scene rendering, GUI)
     * 
     * @return true if all systems initialized successfully
     */
    bool initialize();
    
    /**
     * @brief Start the engine (calls onStart on all systems)
     */
    void start();
    
    /**
     * @brief Update all systems for one frame
     * Called by platform-specific main loop
     */
    void update();
    
    /**
     * @brief Shutdown all systems in reverse order
     */
    void shutdown();

    // ========== System Management ==========
    
    /**
     * @brief Add a system to the engine
     * Systems are automatically sorted by priority after addition
     * 
     * @tparam T System type (must inherit from IEngineSystem)
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Pointer to the created system (non-owning)
     * 
     * Example:
     *   auto* input = engine.addSystem<InputSystem>();
     *   auto* physics = engine.addSystem<PhysicsSystem>(gravity);
     */
    template<typename T, typename... Args>
    T* addSystem(Args&&... args) {
        static_assert(std::is_base_of<IEngineSystem, T>::value, 
                      "T must inherit from IEngineSystem");
        
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* systemPtr = system.get();
        
        systems.push_back(std::move(system));
        
        // Re-sort systems by priority after adding
        sortSystems();
        
        return systemPtr;
    }
    
    /**
     * @brief Get a system by type
     * @tparam T System type
     * @return Pointer to system or nullptr if not found
     * 
     * Example:
     *   auto* input = engine.getSystem<InputSystem>();
     *   if (input) { ... }
     */
    template<typename T>
    T* getSystem() {
        for (auto& system : systems) {
            T* casted = dynamic_cast<T*>(system.get());
            if (casted) {
                return casted;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief Get all systems (for debugging)
     */
    const std::vector<std::unique_ptr<IEngineSystem>>& getSystems() const {
        return systems;
    }

    // ========== Time Management ==========
    
    /**
     * @brief Get time elapsed since last frame (in seconds)
     */
    float getDeltaTime() const { return deltaTime; }
    
    /**
     * @brief Get total running time (in seconds)
     */
    double getTotalTime() const { return totalTime; }
    
    /**
     * @brief Set fixed timestep for physics updates
     */
    void setFixedTimeStep(float step) { fixedTimeStep = step; }
    
    /**
     * @brief Get fixed timestep
     */
    float getFixedTimeStep() const { return fixedTimeStep; }

    // ========== State ==========
    
    /**
     * @brief Check if engine is running
     */
    bool getIsRunning() const { return isRunning; }
    
    /**
     * @brief Stop the engine gracefully
     */
    void stop() { isRunning = false; }

    // ========== Application Control ==========
    
    /**
     * @brief Check if application should close
     * @return true if window close requested
     */
    bool shouldClose() const;

    // ========== Direct System Access ==========
    // Apps should use getSystem<T>() for direct system access.
    // Example:
    //   auto* renderSystem = engine.getSystem<RenderSystem>();
    //   auto* scene = renderSystem->getScene();
    //   scene->createObject("name");
    //   scene->getCamera()->setPosition(pos);
    //
    //   auto* inputSystem = engine.getSystem<InputSystem>();
    //   inputSystem->isKeyPressed(KeyCode::A);
    //
    //   auto* resourceSystem = engine.getSystem<ResourceSystem>();
    //   resourceSystem->createCubeMesh("Cube", 1.0f);

    // ========== Removed Forwarding Methods ==========
    // All input, physics, and resource forwarding methods removed.
    // Use direct system access pattern instead.

private:
    /**
     * @brief Sort systems by priority (lower = earlier)
     */
    void sortSystems();
    
    /**
     * @brief Update delta time
     */
    void updateTime();
    
    /**
     * @brief Process fixed timestep updates
     */
    void updateFixedTimestep();
};

} // namespace rs_engine
