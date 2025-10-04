#pragma once

#include "ResourceTypes.h"
#include "model/Model.h"
#include "model/Mesh.h"
#include "texture/Texture.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <webgpu/webgpu_cpp.h>

namespace rs_engine {
namespace resource {

/**
 * @brief Central resource manager - handles loading, caching, and lifetime
 * 
 * Features:
 * - Resource caching (avoid duplicate loads)
 * - Reference counting
 * - GPU resource management
 * - Memory tracking
 * 
 * Platform Support: 100% shared
 * - Resource loading: identical on all platforms
 * - File system: Web uses virtual FS, Native uses real FS
 */
class ResourceManager {
private:
    // Resource storage
    std::unordered_map<ResourceHandle, std::shared_ptr<IResource>> resources;
    std::unordered_map<std::string, ResourceHandle> nameToHandle;
    std::unordered_map<std::string, ResourceHandle> pathToHandle;
    
    // Handle generation
    ResourceHandle nextHandle = 1;
    
    // WebGPU device for GPU resource creation
    wgpu::Device device;
    
    // Statistics
    size_t totalMemoryUsed = 0;
    size_t gpuMemoryUsed = 0;

public:
    ResourceManager();
    ~ResourceManager();
    
    // ========== Initialization ==========
    
    /**
     * @brief Initialize resource manager with WebGPU device
     * @param wgpuDevice WebGPU device for GPU resource creation
     */
    void initialize(wgpu::Device wgpuDevice);
    
    /**
     * @brief Shutdown and release all resources
     */
    void shutdown();
    
    // ========== Model Management ==========
    
    /**
     * @brief Load a model from file
     * @param name Resource name (for lookup)
     * @param filepath Path to model file (.obj, .gltf, etc.)
     * @return Resource handle (INVALID_RESOURCE_HANDLE on failure)
     */
    ResourceHandle loadModel(const std::string& name, const std::string& filepath);
    
    /**
     * @brief Create a procedural model
     * @param name Resource name
     * @param model Model data
     * @return Resource handle
     */
    ResourceHandle createModel(const std::string& name, std::shared_ptr<Model> model);
    
    /**
     * @brief Get model by handle
     */
    std::shared_ptr<Model> getModel(ResourceHandle handle);
    
    /**
     * @brief Get model by name
     */
    std::shared_ptr<Model> getModel(const std::string& name);
    
    // ========== Mesh Management ==========
    
    /**
     * @brief Create a mesh resource
     * @param name Resource name
     * @param mesh Mesh data
     * @return Resource handle
     */
    ResourceHandle createMesh(const std::string& name, std::shared_ptr<Mesh> mesh);
    
    /**
     * @brief Get mesh by handle
     */
    std::shared_ptr<Mesh> getMesh(ResourceHandle handle);
    
    /**
     * @brief Get mesh by name
     */
    std::shared_ptr<Mesh> getMesh(const std::string& name);
    
    /**
     * @brief Create common procedural meshes
     */
    ResourceHandle createCubeMesh(const std::string& name = "Cube", float size = 1.0f);
    ResourceHandle createSphereMesh(const std::string& name = "Sphere", 
                                   float radius = 1.0f, int segments = 32);
    ResourceHandle createPlaneMesh(const std::string& name = "Plane",
                                  float width = 1.0f, float height = 1.0f);
    
    // ========== Texture Management ==========
    
    /**
     * @brief Load texture from file
     * @param name Resource name
     * @param filepath Path to texture file (.png, .jpg, etc.)
     * @return Resource handle (INVALID_RESOURCE_HANDLE on failure)
     */
    ResourceHandle loadTexture(const std::string& name, const std::string& filepath);
    
    /**
     * @brief Create a texture resource
     * @param name Resource name
     * @param texture Texture data
     * @return Resource handle
     */
    ResourceHandle createTexture(const std::string& name, std::shared_ptr<Texture> texture);
    
    /**
     * @brief Get texture by handle
     */
    std::shared_ptr<Texture> getTexture(ResourceHandle handle);
    
    /**
     * @brief Get texture by name
     */
    std::shared_ptr<Texture> getTexture(const std::string& name);
    
    /**
     * @brief Create common procedural textures
     */
    ResourceHandle createSolidColorTexture(const std::string& name,
                                          uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    ResourceHandle createCheckerboardTexture(const std::string& name,
                                            uint32_t size = 256, uint32_t checkSize = 32);
    
    // ========== Generic Resource Access ==========
    
    /**
     * @brief Get any resource by handle
     */
    std::shared_ptr<IResource> getResource(ResourceHandle handle);
    
    /**
     * @brief Get any resource by name
     */
    std::shared_ptr<IResource> getResource(const std::string& name);
    
    /**
     * @brief Check if resource exists
     */
    bool hasResource(ResourceHandle handle) const;
    bool hasResource(const std::string& name) const;
    
    /**
     * @brief Remove resource (unload and delete)
     */
    void removeResource(ResourceHandle handle);
    void removeResource(const std::string& name);
    
    /**
     * @brief Remove all resources
     */
    void clearAllResources();
    
    // ========== GPU Resource Management ==========
    
    /**
     * @brief Create GPU resources for a specific resource
     */
    bool createGPUResources(ResourceHandle handle);
    
    /**
     * @brief Create GPU resources for all loaded resources
     */
    void createAllGPUResources();
    
    /**
     * @brief Release GPU resources for a specific resource
     */
    void releaseGPUResources(ResourceHandle handle);
    
    /**
     * @brief Release all GPU resources (keep CPU data)
     */
    void releaseAllGPUResources();
    
    // ========== Statistics ==========
    
    /**
     * @brief Get total number of resources
     */
    size_t getResourceCount() const { return resources.size(); }
    
    /**
     * @brief Get total CPU memory used by resources
     */
    size_t getTotalMemoryUsed() const { return totalMemoryUsed; }
    
    /**
     * @brief Get GPU memory used (approximate)
     */
    size_t getGPUMemoryUsed() const { return gpuMemoryUsed; }
    
    /**
     * @brief Print resource statistics
     */
    void printStatistics() const;

private:
    /**
     * @brief Generate unique resource handle
     */
    ResourceHandle generateHandle();
    
    /**
     * @brief Register resource in lookup maps
     */
    void registerResource(std::shared_ptr<IResource> resource, ResourceHandle handle);
    
    /**
     * @brief Unregister resource from lookup maps
     */
    void unregisterResource(ResourceHandle handle);
    
    /**
     * @brief Update memory statistics
     */
    void updateMemoryStats();
};

} // namespace resource
} // namespace rs_engine
