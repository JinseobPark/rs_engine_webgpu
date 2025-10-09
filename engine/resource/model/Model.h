#pragma once

#include "../ResourceTypes.h"
#include "Mesh.h"
#include "../../core/math/Vec3.h"
#include <vector>
#include <memory>
#include <string>

namespace rs_engine {
namespace resource {

/**
 * @brief Transform data for scene object instances
 */
struct Transform {
    Vec3 position = Vec3(0, 0, 0);
    Vec3 rotation = Vec3(0, 0, 0);  // Euler angles in radians
    Vec3 scale = Vec3(1, 1, 1);
    
    Transform() = default;
    Transform(const Vec3& pos, const Vec3& rot = Vec3(0, 0, 0), const Vec3& scl = Vec3(1, 1, 1))
        : position(pos), rotation(rot), scale(scl) {}
};

/**
 * @brief Model resource - collection of meshes (shared resource)
 * 
 * A Model is a shared resource that contains only geometry and material data.
 * It does NOT own a Transform - that belongs to SceneObject instances.
 * 
 * A Model can contain:
 * - Single mesh (simple objects)
 * - Multiple meshes (complex objects with sub-parts)
 * - Hierarchy information (for articulated models)
 * 
 * Architecture:
 * - Model = Shared Resource (geometry + material)
 * - SceneObject = Instance (transform + model reference)
 * 
 * Platform Support: 100% shared
 */
class Model : public IResource {
private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    
    // Bounding information (in model space, origin-centered)
    Vec3 boundingMin;
    Vec3 boundingMax;
    bool boundsDirty = true;

public:
    Model();
    Model(const std::string& name);
    virtual ~Model();
    
    // IResource interface
    bool load() override;
    void unload() override;
    
    // ========== Mesh Management ==========
    
    void addMesh(std::shared_ptr<Mesh> mesh);
    void removeMesh(size_t index);
    void clearMeshes();
    
    size_t getMeshCount() const { return meshes.size(); }
    std::shared_ptr<Mesh> getMesh(size_t index) const;
    const std::vector<std::shared_ptr<Mesh>>& getMeshes() const { return meshes; }
    
    // ========== Bounding Volume (Model Space) ==========
    
    void calculateBounds();
    void getBounds(Vec3& min, Vec3& max);
    
    // ========== GPU Resources ==========
    
    bool createGPUResources(wgpu::Device device);
    void releaseGPUResources();
};

} // namespace resource
} // namespace rs_engine
