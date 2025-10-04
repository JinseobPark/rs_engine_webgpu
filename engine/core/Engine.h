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
    
    /**
     * @brief Get window dimensions
     * @param width Output width
     * @param height Output height
     */
    void getWindowSize(uint32_t& width, uint32_t& height) const;

    // ========== Scene Control ==========
    
    /**
     * @brief Add object to scene
     * @param name Object name
     * @param position World position
     * @param scale Object scale
     */
    void addSceneObject(const std::string& name, 
                       const Vec3& position,
                       const Vec3& scale = Vec3(1.0f, 1.0f, 1.0f));
    
    /**
     * @brief Remove object from scene
     * @param name Object name
     */
    void removeSceneObject(const std::string& name);
    
    /**
     * @brief Clear all scene objects
     */
    void clearScene();

    // ========== Camera Control ==========
    
    /**
     * @brief Set camera position
     * @param position World position
     */
    void setCameraPosition(const Vec3& position);
    
    /**
     * @brief Set camera target
     * @param target Target point to look at
     */
    void setCameraTarget(const Vec3& target);
    
    /**
     * @brief Set camera field of view
     * @param fov Field of view in degrees
     */
    void setCameraFOV(float fov);

    // ========== Input Control ==========
    
    /**
     * @brief Check if key was just pressed this frame
     */
    bool isKeyPressed(int keyCode) const;
    
    /**
     * @brief Check if key is currently held down
     */
    bool isKeyHeld(int keyCode) const;
    
    /**
     * @brief Check if key is down (pressed or held)
     */
    bool isKeyDown(int keyCode) const;
    
    /**
     * @brief Check if mouse button was just pressed this frame
     */
    bool isMouseButtonPressed(int button) const;
    
    /**
     * @brief Check if mouse button is down
     */
    bool isMouseButtonDown(int button) const;
    
    /**
     * @brief Get mouse position
     */
    void getMousePosition(double& x, double& y) const;
    
    /**
     * @brief Get mouse movement delta
     */
    void getMouseDelta(double& dx, double& dy) const;
    
    /**
     * @brief Lock cursor (for FPS camera)
     */
    void lockCursor(bool lock);
    
    /**
     * @brief Show or hide cursor
     */
    void showCursor(bool show);

    // ========== Physics Control ==========
    
    /**
     * @brief Set physics simulation quality
     * @param quality Quality factor (0.0 to 1.0)
     */
    void setPhysicsQuality(float quality);
    
    /**
     * @brief Get physics simulation quality
     * @return Current quality factor
     */
    float getPhysicsQuality() const;
    
    /**
     * @brief Pause or resume physics simulation
     * @param paused true to pause, false to resume
     */
    void setPhysicsPaused(bool paused);
    
    /**
     * @brief Set physics time scale
     * @param scale Time scale factor (1.0 = normal speed)
     */
    void setPhysicsTimeScale(float scale);

    // ========== Resource Control ==========
    
    /**
     * @brief Create procedural mesh resources
     * @param name Resource name
     * @param size/radius/width/height Mesh dimensions
     * @return Resource handle
     */
    uint64_t createCubeMesh(const std::string& name = "Cube", float size = 1.0f);
    uint64_t createSphereMesh(const std::string& name = "Sphere", 
                              float radius = 1.0f, int segments = 32);
    uint64_t createPlaneMesh(const std::string& name = "Plane",
                             float width = 1.0f, float height = 1.0f);
    
    /**
     * @brief Create procedural texture resources
     * @param name Resource name
     * @return Resource handle
     */
    uint64_t createSolidColorTexture(const std::string& name,
                                     uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    uint64_t createCheckerboardTexture(const std::string& name,
                                       uint32_t size = 256, uint32_t checkSize = 32);
    
    /**
     * @brief Load resources from files
     * @param name Resource name
     * @param filepath Path to resource file
     * @return Resource handle
     */
    uint64_t loadModel(const std::string& name, const std::string& filepath);
    uint64_t loadTexture(const std::string& name, const std::string& filepath);
    
    /**
     * @brief Remove resources
     */
    void removeResource(const std::string& name);
    void removeResource(uint64_t handle);
    void clearAllResources();
    
    /**
     * @brief Check if resource exists
     */
    bool hasResource(const std::string& name) const;
    bool hasResource(uint64_t handle) const;
    
    /**
     * @brief Get resource statistics
     */
    size_t getResourceCount() const;
    size_t getResourceMemoryUsed() const;
    size_t getResourceGPUMemoryUsed() const;
    void printResourceStatistics() const;

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
