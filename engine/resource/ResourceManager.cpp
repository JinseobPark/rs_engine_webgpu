#include "ResourceManager.h"
#include <iostream>

namespace rs_engine {
namespace resource {

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
    shutdown();
}

void ResourceManager::initialize(wgpu::Device wgpuDevice) {
    device = wgpuDevice;
    std::cout << "[SUCCESS] ResourceManager initialized" << std::endl;
}

void ResourceManager::shutdown() {
    clearAllResources();
    device = nullptr;
    std::cout << "[INFO] ResourceManager shutdown" << std::endl;
}

// ========== Model Management ==========

ResourceHandle ResourceManager::loadModel(const std::string& name, const std::string& filepath) {
    // Check if already loaded
    auto it = pathToHandle.find(filepath);
    if (it != pathToHandle.end()) {
        std::cout << "[INFO] Model already loaded: " << name << " (" << filepath << ")" << std::endl;
        return it->second;
    }
    
    // TODO: Implement actual model loading with ModelLoader
    std::cerr << "[ERROR] Model loading not yet implemented: " << filepath << std::endl;
    
    return INVALID_RESOURCE_HANDLE;
}

ResourceHandle ResourceManager::createModel(const std::string& name, std::shared_ptr<Model> model) {
    if (!model) {
        return INVALID_RESOURCE_HANDLE;
    }
    
    // Check if name already exists
    if (hasResource(name)) {
        std::cerr << "[WARNING] Model already exists: " << name << std::endl;
        return nameToHandle[name];
    }
    
    ResourceHandle handle = generateHandle();
    model->metadata.handle = handle;
    model->metadata.name = name;
    
    registerResource(model, handle);
    
    // Create GPU resources if device is available
    if (device) {
        model->createGPUResources(device);
    }
    
    updateMemoryStats();
    
    std::cout << "[SUCCESS] Model created: " << name << " (Handle: " << handle << ")" << std::endl;
    return handle;
}

std::shared_ptr<Model> ResourceManager::getModel(ResourceHandle handle) {
    auto resource = getResource(handle);
    return std::dynamic_pointer_cast<Model>(resource);
}

std::shared_ptr<Model> ResourceManager::getModel(const std::string& name) {
    auto resource = getResource(name);
    return std::dynamic_pointer_cast<Model>(resource);
}

// ========== Mesh Management ==========

ResourceHandle ResourceManager::createMesh(const std::string& name, std::shared_ptr<Mesh> mesh) {
    if (!mesh) {
        return INVALID_RESOURCE_HANDLE;
    }
    
    if (hasResource(name)) {
        std::cerr << "[WARNING] Mesh already exists: " << name << std::endl;
        return nameToHandle[name];
    }
    
    ResourceHandle handle = generateHandle();
    mesh->metadata.handle = handle;
    mesh->metadata.name = name;
    
    registerResource(mesh, handle);
    
    if (device) {
        mesh->createGPUResources(device);
    }
    
    updateMemoryStats();
    
    std::cout << "[SUCCESS] Mesh created: " << name << " (Handle: " << handle << ")" << std::endl;
    return handle;
}

std::shared_ptr<Mesh> ResourceManager::getMesh(ResourceHandle handle) {
    auto resource = getResource(handle);
    return std::dynamic_pointer_cast<Mesh>(resource);
}

std::shared_ptr<Mesh> ResourceManager::getMesh(const std::string& name) {
    auto resource = getResource(name);
    return std::dynamic_pointer_cast<Mesh>(resource);
}

ResourceHandle ResourceManager::createCubeMesh(const std::string& name, float size) {
    auto mesh = std::shared_ptr<Mesh>(Mesh::createCube(name, size));
    return createMesh(name, mesh);
}

ResourceHandle ResourceManager::createSphereMesh(const std::string& name, float radius, int segments) {
    auto mesh = std::shared_ptr<Mesh>(Mesh::createSphere(name, radius, segments));
    return createMesh(name, mesh);
}

ResourceHandle ResourceManager::createPlaneMesh(const std::string& name, float width, float height) {
    auto mesh = std::shared_ptr<Mesh>(Mesh::createPlane(name, width, height));
    return createMesh(name, mesh);
}

// ========== Texture Management ==========

ResourceHandle ResourceManager::loadTexture(const std::string& name, const std::string& filepath) {
    auto it = pathToHandle.find(filepath);
    if (it != pathToHandle.end()) {
        std::cout << "[INFO] Texture already loaded: " << name << " (" << filepath << ")" << std::endl;
        return it->second;
    }
    
    auto texture = std::make_shared<Texture>(name);
    texture->metadata.filepath = filepath;
    
    if (!texture->loadFromFile(filepath)) {
        std::cerr << "[ERROR] Failed to load texture: " << filepath << std::endl;
        return INVALID_RESOURCE_HANDLE;
    }
    
    ResourceHandle handle = generateHandle();
    texture->metadata.handle = handle;
    
    registerResource(texture, handle);
    
    if (device) {
        texture->createGPUResources(device);
    }
    
    updateMemoryStats();
    
    std::cout << "[SUCCESS] Texture loaded: " << name << " (" << filepath << ")" << std::endl;
    return handle;
}

ResourceHandle ResourceManager::createTexture(const std::string& name, std::shared_ptr<Texture> texture) {
    if (!texture) {
        return INVALID_RESOURCE_HANDLE;
    }
    
    if (hasResource(name)) {
        std::cerr << "[WARNING] Texture already exists: " << name << std::endl;
        return nameToHandle[name];
    }
    
    ResourceHandle handle = generateHandle();
    texture->metadata.handle = handle;
    texture->metadata.name = name;
    
    registerResource(texture, handle);
    
    if (device) {
        texture->createGPUResources(device);
    }
    
    updateMemoryStats();
    
    std::cout << "[SUCCESS] Texture created: " << name << " (Handle: " << handle << ")" << std::endl;
    return handle;
}

std::shared_ptr<Texture> ResourceManager::getTexture(ResourceHandle handle) {
    auto resource = getResource(handle);
    return std::dynamic_pointer_cast<Texture>(resource);
}

std::shared_ptr<Texture> ResourceManager::getTexture(const std::string& name) {
    auto resource = getResource(name);
    return std::dynamic_pointer_cast<Texture>(resource);
}

ResourceHandle ResourceManager::createSolidColorTexture(const std::string& name,
                                                       uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    auto texture = std::shared_ptr<Texture>(Texture::createSolidColor(name, r, g, b, a));
    return createTexture(name, texture);
}

ResourceHandle ResourceManager::createCheckerboardTexture(const std::string& name,
                                                         uint32_t size, uint32_t checkSize) {
    auto texture = std::shared_ptr<Texture>(Texture::createCheckerboard(name, size, checkSize));
    return createTexture(name, texture);
}

// ========== Generic Resource Access ==========

std::shared_ptr<IResource> ResourceManager::getResource(ResourceHandle handle) {
    auto it = resources.find(handle);
    if (it != resources.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<IResource> ResourceManager::getResource(const std::string& name) {
    auto it = nameToHandle.find(name);
    if (it != nameToHandle.end()) {
        return getResource(it->second);
    }
    return nullptr;
}

bool ResourceManager::hasResource(ResourceHandle handle) const {
    return resources.find(handle) != resources.end();
}

bool ResourceManager::hasResource(const std::string& name) const {
    return nameToHandle.find(name) != nameToHandle.end();
}

void ResourceManager::removeResource(ResourceHandle handle) {
    auto it = resources.find(handle);
    if (it != resources.end()) {
        unregisterResource(handle);
        it->second->unload();
        resources.erase(it);
        updateMemoryStats();
        
        std::cout << "[INFO] Resource removed (Handle: " << handle << ")" << std::endl;
    }
}

void ResourceManager::removeResource(const std::string& name) {
    auto it = nameToHandle.find(name);
    if (it != nameToHandle.end()) {
        removeResource(it->second);
    }
}

void ResourceManager::clearAllResources() {
    std::cout << "[INFO] Clearing all resources (" << resources.size() << " total)" << std::endl;
    
    for (auto& pair : resources) {
        pair.second->unload();
    }
    
    resources.clear();
    nameToHandle.clear();
    pathToHandle.clear();
    nextHandle = 1;
    totalMemoryUsed = 0;
    gpuMemoryUsed = 0;
}

// ========== GPU Resource Management ==========

bool ResourceManager::createGPUResources(ResourceHandle handle) {
    if (!device) {
        std::cerr << "[ERROR] No device available for GPU resource creation" << std::endl;
        return false;
    }
    
    auto resource = getResource(handle);
    if (!resource) {
        return false;
    }
    
    // Handle different resource types
    if (auto model = std::dynamic_pointer_cast<Model>(resource)) {
        return model->createGPUResources(device);
    } else if (auto mesh = std::dynamic_pointer_cast<Mesh>(resource)) {
        return mesh->createGPUResources(device);
    } else if (auto texture = std::dynamic_pointer_cast<Texture>(resource)) {
        return texture->createGPUResources(device);
    }
    
    return false;
}

void ResourceManager::createAllGPUResources() {
    if (!device) {
        std::cerr << "[ERROR] No device available for GPU resource creation" << std::endl;
        return;
    }
    
    std::cout << "[INFO] Creating GPU resources for all loaded resources..." << std::endl;
    
    int successCount = 0;
    int failCount = 0;
    
    for (auto& pair : resources) {
        if (createGPUResources(pair.first)) {
            successCount++;
        } else {
            failCount++;
        }
    }
    
    std::cout << "[SUCCESS] GPU resources created: " << successCount << " succeeded, " 
              << failCount << " failed" << std::endl;
}

void ResourceManager::releaseGPUResources(ResourceHandle handle) {
    auto resource = getResource(handle);
    if (!resource) {
        return;
    }
    
    if (auto model = std::dynamic_pointer_cast<Model>(resource)) {
        model->releaseGPUResources();
    } else if (auto mesh = std::dynamic_pointer_cast<Mesh>(resource)) {
        mesh->releaseGPUResources();
    } else if (auto texture = std::dynamic_pointer_cast<Texture>(resource)) {
        texture->releaseGPUResources();
    }
}

void ResourceManager::releaseAllGPUResources() {
    std::cout << "[INFO] Releasing all GPU resources..." << std::endl;
    
    for (auto& pair : resources) {
        releaseGPUResources(pair.first);
    }
}

// ========== Statistics ==========

void ResourceManager::printStatistics() const {
    std::cout << "\n========== Resource Manager Statistics ==========" << std::endl;
    std::cout << "Total Resources: " << resources.size() << std::endl;
    std::cout << "CPU Memory Used: " << (totalMemoryUsed / 1024.0 / 1024.0) << " MB" << std::endl;
    std::cout << "GPU Memory Used: " << (gpuMemoryUsed / 1024.0 / 1024.0) << " MB (estimate)" << std::endl;
    
    // Count by type
    int modelCount = 0, meshCount = 0, textureCount = 0, otherCount = 0;
    
    for (const auto& pair : resources) {
        switch (pair.second->getType()) {
            case ResourceType::Model: modelCount++; break;
            case ResourceType::Mesh: meshCount++; break;
            case ResourceType::Texture: textureCount++; break;
            default: otherCount++; break;
        }
    }
    
    std::cout << "\nBy Type:" << std::endl;
    std::cout << "  Models: " << modelCount << std::endl;
    std::cout << "  Meshes: " << meshCount << std::endl;
    std::cout << "  Textures: " << textureCount << std::endl;
    std::cout << "  Other: " << otherCount << std::endl;
    std::cout << "================================================\n" << std::endl;
}

// ========== Private Methods ==========

ResourceHandle ResourceManager::generateHandle() {
    return nextHandle++;
}

void ResourceManager::registerResource(std::shared_ptr<IResource> resource, ResourceHandle handle) {
    resources[handle] = resource;
    
    if (!resource->getName().empty()) {
        nameToHandle[resource->getName()] = handle;
    }
    
    if (!resource->getFilePath().empty()) {
        pathToHandle[resource->getFilePath()] = handle;
    }
}

void ResourceManager::unregisterResource(ResourceHandle handle) {
    auto it = resources.find(handle);
    if (it != resources.end()) {
        auto& resource = it->second;
        
        // Remove from name map
        auto nameIt = nameToHandle.find(resource->getName());
        if (nameIt != nameToHandle.end() && nameIt->second == handle) {
            nameToHandle.erase(nameIt);
        }
        
        // Remove from path map
        auto pathIt = pathToHandle.find(resource->getFilePath());
        if (pathIt != pathToHandle.end() && pathIt->second == handle) {
            pathToHandle.erase(pathIt);
        }
    }
}

void ResourceManager::updateMemoryStats() {
    totalMemoryUsed = 0;
    gpuMemoryUsed = 0;
    
    for (const auto& pair : resources) {
        totalMemoryUsed += pair.second->getMemorySize();
        
        // Estimate GPU memory (rough approximation)
        if (auto mesh = std::dynamic_pointer_cast<Mesh>(pair.second)) {
            if (mesh->hasGPUResources()) {
                gpuMemoryUsed += mesh->getMemorySize();
            }
        } else if (auto texture = std::dynamic_pointer_cast<Texture>(pair.second)) {
            if (texture->hasGPUResources()) {
                gpuMemoryUsed += texture->getMemorySize();
            }
        }
    }
}

} // namespace resource
} // namespace rs_engine
