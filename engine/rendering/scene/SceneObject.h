#pragma once

#include "../../core/math/Mat4.h"
#include "../../core/math/Vec3.h"
#include "../../resource/model/Model.h"
#include <memory>
#include <string>

namespace rs_engine {
namespace rendering {

/**
 * @brief Scene Object - An instance of a Model in the 3D scene
 * 
 * Architecture:
 * - SceneObject owns Transform (position, rotation, scale)
 * - SceneObject references Model (shared resource: geometry + material)
 * - Multiple SceneObjects can share the same Model with different transforms
 * 
 * Example:
 *   auto cubeModel = resourceManager->getModel("cube");
 *   auto obj1 = std::make_unique<SceneObject>("Cube1");
 *   obj1->setModel(cubeModel);
 *   obj1->setPosition(Vec3(0, 0, 0));  // Independent transform
 * 
 *   auto obj2 = std::make_unique<SceneObject>("Cube2");
 *   obj2->setModel(cubeModel);           // Same model
 *   obj2->setPosition(Vec3(5, 0, 0));    // Different transform
 * 
 * This follows the Unity/Unreal pattern: GameObject + MeshRenderer
 */
class SceneObject {
private:
    std::string name;
    resource::Transform transform;              // ✅ SceneObject owns Transform
    std::shared_ptr<resource::Model> model;     // ✅ Shared Model reference
    float animationTime = 0.0f;
    bool isVisible = true;
    bool isSelected = false;  // Selection state for picking

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
    
    void update(float deltaTime) { animationTime += deltaTime; }
    float getAnimationTime() const { return animationTime; }
    void setAnimationTime(float time) { animationTime = time; }

    // ========== Visibility ==========
    
    void setVisible(bool visible) { isVisible = visible; }
    bool getVisible() const { return isVisible; }
    
    // ========== Selection ==========
    
    void setSelected(bool selected) { isSelected = selected; }
    bool getSelected() const { return isSelected; }
    
    // ========== Bounding Volume ==========
    
    /**
     * @brief Get world-space axis-aligned bounding box
     * @param min Output: minimum corner in world space
     * @param max Output: maximum corner in world space
     */
    void getWorldBounds(Vec3& min, Vec3& max) const;
};

} // namespace rendering
} // namespace rs_engine
