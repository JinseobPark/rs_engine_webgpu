#pragma once

#include "../../core/IEngineSystem.h"
#include "../../resource/ResourceManager.h"
#include <memory>

namespace rs_engine {

// Forward declaration
class ApplicationSystem;

/**
 * @brief Resource System - Manages model, texture, and material resources
 * 
 * Responsibilities:
 * - Resource loading and caching
 * - GPU resource management
 * - Memory tracking
 * - Asset lifecycle management
 * 
 * Platform Support: 95% shared
 * - Resource management: 100% shared
 * - File loading: Web uses virtual FS, Native uses real FS
 * 
 * Priority: -75 (before Input/Physics/Render, after Application)
 */
class ResourceSystem : public IEngineSystem {
private:
    ApplicationSystem* appSystem = nullptr;
    std::unique_ptr<resource::ResourceManager> resourceManager;
    
    bool enabled = true;

public:
    ResourceSystem() = default;
    virtual ~ResourceSystem() = default;

    // ========== IEngineSystem Interface ==========
    
    bool initialize(Engine* engineRef) override;
    void onStart() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    const char* getName() const override { return "Resource"; }
    int getPriority() const override { return -75; }
    
    void setEnabled(bool value) override { enabled = value; }
    bool isEnabled() const override { return enabled; }
    
    // ========== Resource Manager Access ==========
    
    /**
     * @brief Get the resource manager instance
     * @return ResourceManager pointer (non-owning)
     */
    resource::ResourceManager* getResourceManager() { return resourceManager.get(); }
    
    // ========== Model Management ==========
    
    /**
     * @brief Load a model from file
     * @param name Resource name
     * @param filepath Path to model file
     * @return Resource handle
     */
    resource::ResourceHandle loadModel(const std::string& name, const std::string& filepath);
    
    /**
     * @brief Create a procedural model
     * @param name Resource name
     * @param model Model data
     * @return Resource handle
     */
    resource::ResourceHandle createModel(const std::string& name, std::shared_ptr<resource::Model> model);
    
    /**
     * @brief Get model resource
     */
    std::shared_ptr<resource::Model> getModel(const std::string& name);
    std::shared_ptr<resource::Model> getModel(resource::ResourceHandle handle);
    
    // ========== Mesh Management ==========
    
    /**
     * @brief Create procedural meshes
     */
    resource::ResourceHandle createCubeMesh(const std::string& name = "Cube", float size = 1.0f);
    resource::ResourceHandle createSphereMesh(const std::string& name = "Sphere", 
                                             float radius = 1.0f, int segments = 32);
    resource::ResourceHandle createPlaneMesh(const std::string& name = "Plane",
                                            float width = 1.0f, float height = 1.0f);
    
    /**
     * @brief Get mesh resource
     */
    std::shared_ptr<resource::Mesh> getMesh(const std::string& name);
    std::shared_ptr<resource::Mesh> getMesh(resource::ResourceHandle handle);
    
    // ========== Texture Management ==========
    
    /**
     * @brief Load a texture from file
     * @param name Resource name
     * @param filepath Path to texture file
     * @return Resource handle
     */
    resource::ResourceHandle loadTexture(const std::string& name, const std::string& filepath);
    
    /**
     * @brief Create procedural textures
     */
    resource::ResourceHandle createSolidColorTexture(const std::string& name,
                                                     uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    resource::ResourceHandle createCheckerboardTexture(const std::string& name,
                                                       uint32_t size = 256, uint32_t checkSize = 32);
    
    /**
     * @brief Get texture resource
     */
    std::shared_ptr<resource::Texture> getTexture(const std::string& name);
    std::shared_ptr<resource::Texture> getTexture(resource::ResourceHandle handle);
    
    // ========== Resource Management ==========
    
    /**
     * @brief Remove a resource
     */
    void removeResource(const std::string& name);
    void removeResource(resource::ResourceHandle handle);
    
    /**
     * @brief Remove all resources
     */
    void clearAllResources();
    
    /**
     * @brief Check if resource exists
     */
    bool hasResource(const std::string& name) const;
    bool hasResource(resource::ResourceHandle handle) const;
    
    // ========== Statistics ==========
    
    /**
     * @brief Get resource count
     */
    size_t getResourceCount() const;
    
    /**
     * @brief Get memory usage
     */
    size_t getTotalMemoryUsed() const;
    size_t getGPUMemoryUsed() const;
    
    /**
     * @brief Print resource statistics
     */
    void printStatistics() const;
};

} // namespace rs_engine
