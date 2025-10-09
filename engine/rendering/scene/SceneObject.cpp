#include "SceneObject.h"
#include <limits>
#include <algorithm>

namespace rs_engine {
namespace rendering {

Mat4 SceneObject::getModelMatrix() const {
    // Create transformation matrices
    Mat4 translationMat = Mat4::translation(transform.position);
    Mat4 rotationMat = Mat4::rotationY(animationTime); // Simple Y-axis rotation for now
    Mat4 scaleMat = Mat4::scale(transform.scale);

    // Combine transformations: translation * rotation * scale
    return translationMat * rotationMat * scaleMat;
}

void SceneObject::getWorldBounds(Vec3& min, Vec3& max) const {
    if (!model) {
        // No model - return point at object position
        min = transform.position;
        max = transform.position;
        return;
    }
    
    // Get model-space bounds
    Vec3 modelMin, modelMax;
    const_cast<resource::Model*>(model.get())->getBounds(modelMin, modelMax);
    
    // Get model matrix
    Mat4 modelMatrix = getModelMatrix();
    
    // Transform all 8 corners of the bounding box to world space
    Vec3 corners[8] = {
        Vec3(modelMin.x, modelMin.y, modelMin.z),
        Vec3(modelMin.x, modelMin.y, modelMax.z),
        Vec3(modelMin.x, modelMax.y, modelMin.z),
        Vec3(modelMin.x, modelMax.y, modelMax.z),
        Vec3(modelMax.x, modelMin.y, modelMin.z),
        Vec3(modelMax.x, modelMin.y, modelMax.z),
        Vec3(modelMax.x, modelMax.y, modelMin.z),
        Vec3(modelMax.x, modelMax.y, modelMax.z)
    };
    
    // Find world-space min/max
    min = Vec3(std::numeric_limits<float>::max(), 
               std::numeric_limits<float>::max(), 
               std::numeric_limits<float>::max());
    max = Vec3(std::numeric_limits<float>::lowest(), 
               std::numeric_limits<float>::lowest(), 
               std::numeric_limits<float>::lowest());
    
    for (int i = 0; i < 8; i++) {
        Vec3 worldCorner = modelMatrix.transformPoint(corners[i]);
        min.x = std::min(min.x, worldCorner.x);
        min.y = std::min(min.y, worldCorner.y);
        min.z = std::min(min.z, worldCorner.z);
        max.x = std::max(max.x, worldCorner.x);
        max.y = std::max(max.y, worldCorner.y);
        max.z = std::max(max.z, worldCorner.z);
    }
}

} // namespace rendering
} // namespace rs_engine
