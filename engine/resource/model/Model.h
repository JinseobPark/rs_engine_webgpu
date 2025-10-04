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
 * @brief Transform data for model instances
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
 * @brief Model resource - collection of meshes with transform
 * 
 * A Model can contain:
 * - Single mesh (simple objects)
 * - Multiple meshes (complex objects with sub-parts)
 * - Hierarchy information (for articulated models)
 * 
 * Platform Support: 100% shared
 */
class Model : public IResource {
private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    Transform transform;
    
    // Bounding information
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
    
    // ========== Transform ==========
    
    void setTransform(const Transform& trans) { transform = trans; }
    const Transform& getTransform() const { return transform; }
    Transform& getTransform() { return transform; }
    
    void setPosition(const Vec3& pos) { transform.position = pos; }
    void setRotation(const Vec3& rot) { transform.rotation = rot; }
    void setScale(const Vec3& scl) { transform.scale = scl; }
    
    const Vec3& getPosition() const { return transform.position; }
    const Vec3& getRotation() const { return transform.rotation; }
    const Vec3& getScale() const { return transform.scale; }
    
    // ========== Bounding Volume ==========
    
    void calculateBounds();
    void getBounds(Vec3& min, Vec3& max);
    
    // ========== GPU Resources ==========
    
    bool createGPUResources(wgpu::Device device);
    void releaseGPUResources();
};

} // namespace resource
} // namespace rs_engine
