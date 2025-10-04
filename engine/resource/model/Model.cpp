#include "Model.h"
#include <algorithm>
#include <limits>

namespace rs_engine {
namespace resource {

Model::Model() {
    metadata.type = ResourceType::Model;
    metadata.state = ResourceState::Unloaded;
}

Model::Model(const std::string& name) : Model() {
    metadata.name = name;
}

Model::~Model() {
    unload();
}

bool Model::load() {
    if (metadata.state == ResourceState::Loaded) {
        return true;
    }
    
    if (meshes.empty()) {
        metadata.state = ResourceState::Failed;
        return false;
    }
    
    // Load all meshes
    bool allLoaded = true;
    size_t totalMemory = 0;
    
    for (auto& mesh : meshes) {
        if (!mesh->load()) {
            allLoaded = false;
        }
        totalMemory += mesh->getMemorySize();
    }
    
    if (allLoaded) {
        metadata.state = ResourceState::Loaded;
        metadata.memorySize = totalMemory;
        calculateBounds();
        return true;
    }
    
    metadata.state = ResourceState::Failed;
    return false;
}

void Model::unload() {
    releaseGPUResources();
    
    for (auto& mesh : meshes) {
        mesh->unload();
    }
    
    meshes.clear();
    metadata.state = ResourceState::Unloaded;
    metadata.memorySize = 0;
}

void Model::addMesh(std::shared_ptr<Mesh> mesh) {
    if (mesh) {
        meshes.push_back(mesh);
        boundsDirty = true;
    }
}

void Model::removeMesh(size_t index) {
    if (index < meshes.size()) {
        meshes.erase(meshes.begin() + index);
        boundsDirty = true;
    }
}

void Model::clearMeshes() {
    meshes.clear();
    boundsDirty = true;
}

std::shared_ptr<Mesh> Model::getMesh(size_t index) const {
    if (index < meshes.size()) {
        return meshes[index];
    }
    return nullptr;
}

void Model::calculateBounds() {
    if (meshes.empty()) {
        boundingMin = Vec3(0, 0, 0);
        boundingMax = Vec3(0, 0, 0);
        boundsDirty = false;
        return;
    }
    
    float maxFloat = std::numeric_limits<float>::max();
    float minFloat = std::numeric_limits<float>::lowest();
    boundingMin = Vec3(maxFloat, maxFloat, maxFloat);
    boundingMax = Vec3(minFloat, minFloat, minFloat);
    
    for (const auto& mesh : meshes) {
        Vec3 meshMin, meshMax;
        mesh->calculateBounds(meshMin, meshMax);
        
        boundingMin.x = std::min(boundingMin.x, meshMin.x);
        boundingMin.y = std::min(boundingMin.y, meshMin.y);
        boundingMin.z = std::min(boundingMin.z, meshMin.z);
        
        boundingMax.x = std::max(boundingMax.x, meshMax.x);
        boundingMax.y = std::max(boundingMax.y, meshMax.y);
        boundingMax.z = std::max(boundingMax.z, meshMax.z);
    }
    
    boundsDirty = false;
}

void Model::getBounds(Vec3& min, Vec3& max) {
    if (boundsDirty) {
        calculateBounds();
    }
    
    min = boundingMin;
    max = boundingMax;
}

bool Model::createGPUResources(wgpu::Device device) {
    if (!device) {
        return false;
    }
    
    bool allCreated = true;
    
    for (auto& mesh : meshes) {
        if (!mesh->createGPUResources(device)) {
            allCreated = false;
        }
    }
    
    return allCreated;
}

void Model::releaseGPUResources() {
    for (auto& mesh : meshes) {
        mesh->releaseGPUResources();
    }
}

} // namespace resource
} // namespace rs_engine
