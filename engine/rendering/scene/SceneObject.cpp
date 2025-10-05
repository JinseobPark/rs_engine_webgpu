#include "SceneObject.h"

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

} // namespace rendering
} // namespace rs_engine
