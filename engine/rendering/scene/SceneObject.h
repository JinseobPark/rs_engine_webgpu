#pragma once

#include "../../core/math/Mat4.h"
#include "../../core/math/Vec3.h"
#include "../../resource/model/Model.h"
#include <memory>
#include <string>

namespace rs_engine {
namespace rendering {

/**
 * @brief Scene Object - An object in the 3D scene
 * 
 * A SceneObject represents an entity in the scene with:
 * - Transform (position, rotation, scale)
 * - Model reference (from ResourceManager)
 * - Animation state
 * 
 * This replaces the temporary CubeObject class with a proper
 * resource-based architecture.
 */
class SceneObject {
private:
    std::string name;
    resource::Transform transform;
    std::shared_ptr<resource::Model> model;
    float animationTime = 0.0f;
    bool isVisible = true;

public:
    SceneObject(const std::string& objName = "Object")
        : name(objName) {}

    // ========== Identity ==========
    
    void setName(const std::string& objName) { name = objName; }
    const std::string& getName() const { return name; }

    // ========== Transform ==========
    
    void setTransform(const resource::Transform& trans) { transform = trans; }
    const resource::Transform& getTransform() const { return transform; }
    resource::Transform& getTransform() { return transform; }
    
    void setPosition(const Vec3& pos) { transform.position = pos; }
    void setRotation(const Vec3& rot) { transform.rotation = rot; }
    void setScale(const Vec3& scale) { transform.scale = scale; }
    
    const Vec3& getPosition() const { return transform.position; }
    const Vec3& getRotation() const { return transform.rotation; }
    const Vec3& getScale() const { return transform.scale; }
    
    Mat4 getModelMatrix() const;

    // ========== Model ==========
    
    void setModel(std::shared_ptr<resource::Model> mdl) { model = mdl; }
    std::shared_ptr<resource::Model> getModel() const { return model; }
    bool hasModel() const { return model != nullptr; }

    // ========== Animation ==========
    
    void update(float deltaTime) { /* animationTime += deltaTime; */ }
    float getAnimationTime() const { return animationTime; }
    void setAnimationTime(float time) { animationTime = time; }

    // ========== Visibility ==========
    
    void setVisible(bool visible) { isVisible = visible; }
    bool getVisible() const { return isVisible; }
};

} // namespace rendering
} // namespace rs_engine
