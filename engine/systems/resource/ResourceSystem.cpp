#include "ResourceSystem.h"
#include "../../core/Engine.h"
#include "../application/ApplicationSystem.h"
#include <iostream>

namespace rs_engine {

bool ResourceSystem::initialize(Engine* engineRef) {
    if (!IEngineSystem::initialize(engineRef)) {
        return false;
    }
    
    std::cout << "   🔧 Initializing Resource (priority: " << getPriority() << ")..." << std::endl;
    std::cout << "🎯 Initializing Resource System..." << std::endl;
    
    // Get ApplicationSystem for WebGPU device
    appSystem = engine->getSystem<ApplicationSystem>();
    if (!appSystem) {
        std::cerr << "❌ ApplicationSystem not found! ResourceSystem requires ApplicationSystem." << std::endl;
        return false;
    }
    
    // Create resource manager
    resourceManager = std::make_unique<resource::ResourceManager>();
    
    // Initialize with WebGPU device
    resourceManager->initialize(appSystem->getDevice());
    
    initialized = true;
    std::cout << "✅ Resource System initialized" << std::endl;
    std::cout << "   ✅ Resource initialized" << std::endl;
    
    return true;
}

void ResourceSystem::onStart() {
    std::cout << "[Resource] Started" << std::endl;
}

void ResourceSystem::onUpdate(float deltaTime) {
    // Resource system is mostly passive
    // Can be used for async loading updates in the future
}

void ResourceSystem::onShutdown() {
    std::cout << "🔌 Shutting down Resource System..." << std::endl;
    
    if (resourceManager) {
        resourceManager->shutdown();
        resourceManager.reset();
    }
    
    std::cout << "✅ Resource System shutdown complete" << std::endl;
}

// ========== Model Management ==========

resource::ResourceHandle ResourceSystem::loadModel(const std::string& name, const std::string& filepath) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->loadModel(name, filepath);
}

resource::ResourceHandle ResourceSystem::createModel(const std::string& name, 
                                                    std::shared_ptr<resource::Model> model) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->createModel(name, model);
}

std::shared_ptr<resource::Model> ResourceSystem::getModel(const std::string& name) {
    if (!resourceManager) {
        return nullptr;
    }
    return resourceManager->getModel(name);
}

std::shared_ptr<resource::Model> ResourceSystem::getModel(resource::ResourceHandle handle) {
    if (!resourceManager) {
        return nullptr;
    }
    return resourceManager->getModel(handle);
}

// ========== Mesh Management ==========

resource::ResourceHandle ResourceSystem::createCubeMesh(const std::string& name, float size) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->createCubeMesh(name, size);
}

resource::ResourceHandle ResourceSystem::createSphereMesh(const std::string& name, 
                                                         float radius, int segments) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->createSphereMesh(name, radius, segments);
}

resource::ResourceHandle ResourceSystem::createPlaneMesh(const std::string& name,
                                                        float width, float height) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->createPlaneMesh(name, width, height);
}

std::shared_ptr<resource::Mesh> ResourceSystem::getMesh(const std::string& name) {
    if (!resourceManager) {
        return nullptr;
    }
    return resourceManager->getMesh(name);
}

std::shared_ptr<resource::Mesh> ResourceSystem::getMesh(resource::ResourceHandle handle) {
    if (!resourceManager) {
        return nullptr;
    }
    return resourceManager->getMesh(handle);
}

// ========== Texture Management ==========

resource::ResourceHandle ResourceSystem::loadTexture(const std::string& name, const std::string& filepath) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->loadTexture(name, filepath);
}

resource::ResourceHandle ResourceSystem::createSolidColorTexture(const std::string& name,
                                                                 uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->createSolidColorTexture(name, r, g, b, a);
}

resource::ResourceHandle ResourceSystem::createCheckerboardTexture(const std::string& name,
                                                                   uint32_t size, uint32_t checkSize) {
    if (!resourceManager) {
        return resource::INVALID_RESOURCE_HANDLE;
    }
    return resourceManager->createCheckerboardTexture(name, size, checkSize);
}

std::shared_ptr<resource::Texture> ResourceSystem::getTexture(const std::string& name) {
    if (!resourceManager) {
        return nullptr;
    }
    return resourceManager->getTexture(name);
}

std::shared_ptr<resource::Texture> ResourceSystem::getTexture(resource::ResourceHandle handle) {
    if (!resourceManager) {
        return nullptr;
    }
    return resourceManager->getTexture(handle);
}

// ========== Resource Management ==========

void ResourceSystem::removeResource(const std::string& name) {
    if (resourceManager) {
        resourceManager->removeResource(name);
    }
}

void ResourceSystem::removeResource(resource::ResourceHandle handle) {
    if (resourceManager) {
        resourceManager->removeResource(handle);
    }
}

void ResourceSystem::clearAllResources() {
    if (resourceManager) {
        resourceManager->clearAllResources();
    }
}

bool ResourceSystem::hasResource(const std::string& name) const {
    if (!resourceManager) {
        return false;
    }
    return resourceManager->hasResource(name);
}

bool ResourceSystem::hasResource(resource::ResourceHandle handle) const {
    if (!resourceManager) {
        return false;
    }
    return resourceManager->hasResource(handle);
}

// ========== Statistics ==========

size_t ResourceSystem::getResourceCount() const {
    if (!resourceManager) {
        return 0;
    }
    return resourceManager->getResourceCount();
}

size_t ResourceSystem::getTotalMemoryUsed() const {
    if (!resourceManager) {
        return 0;
    }
    return resourceManager->getTotalMemoryUsed();
}

size_t ResourceSystem::getGPUMemoryUsed() const {
    if (!resourceManager) {
        return 0;
    }
    return resourceManager->getGPUMemoryUsed();
}

void ResourceSystem::printStatistics() const {
    if (resourceManager) {
        resourceManager->printStatistics();
    }
}

} // namespace rs_engine
