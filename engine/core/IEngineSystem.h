#pragma once

namespace rs_engine {

// Forward declaration
class Engine;

/**
 * @brief Base interface for all engine systems
 * 
 * This interface provides a unified lifecycle for all engine subsystems.
 * Systems are initialized in priority order and updated every frame.
 * 
 * Platform Support: 100% shared between Web and Native
 */
class IEngineSystem {
    friend class Engine; // Allow Engine to access protected members

protected:
    Engine* engine = nullptr;
    bool initialized = false;

public:
    virtual ~IEngineSystem() = default;

    /**
     * @brief Initialize the system with engine reference
     * @param engineRef Pointer to the main engine instance
     * @return true if initialization succeeded
     */
    virtual bool initialize(Engine* engineRef) {
        engine = engineRef;
        return true;
    }

    /**
     * @brief Called once after all systems are initialized
     * Use this for setup that requires other systems to be ready
     */
    virtual void onStart() {}

    /**
     * @brief Update the system every frame
     * @param deltaTime Time elapsed since last frame in seconds
     */
    virtual void onUpdate(float deltaTime) = 0;

    /**
     * @brief Fixed timestep update (optional)
     * Override this for physics or other fixed-rate updates
     * @param fixedDeltaTime Fixed time step (e.g., 1/60 = 0.0166s)
     */
    virtual void onFixedUpdate(float fixedDeltaTime) {}

    /**
     * @brief Cleanup resources before shutdown
     */
    virtual void onShutdown() {}

    /**
     * @brief Get system name for debugging
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Get execution priority (lower values run first)
     * 
     * Typical priorities:
     * -100: Platform/Application (window, events)
     * -50:  Input
     * 0:    Game Logic (default)
     * 50:   Physics
     * 100:  Rendering
     */
    virtual int getPriority() const { return 0; }

    /**
     * @brief Check if system is initialized
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Enable/disable system updates
     */
    virtual void setEnabled(bool enabled) {}
    virtual bool isEnabled() const { return true; }
};

} // namespace rs_engine
