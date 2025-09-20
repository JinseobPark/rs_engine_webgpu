#pragma once

#include "../../core/math/Mat4.h"
#include "../../core/math/Vec3.h"
#include "Camera.h"
#include "../ShaderManager.h"
#include <memory>
#include <vector>

#ifdef __EMSCRIPTEN__
    #include <webgpu/webgpu.h>
    #include <webgpu/webgpu_cpp.h>
#else
    #include <dawn/webgpu_cpp.h>
#endif

namespace rs_engine {

// Reuse the Mat4 structure and CubeUniforms from CubeRenderer
struct CubeUniforms {
    Mat4 viewProj;
    Mat4 model;
    float time;
    float padding[3]; // Ensure 16-byte alignment
};

namespace rendering {

// Simple cube object that can be rendered in the scene
class CubeObject {
private:
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    float animationTime;

public:
    CubeObject(const Vec3& pos = Vec3(0, 0, 0), const Vec3& rot = Vec3(0, 0, 0), const Vec3& scale = Vec3(1, 1, 1))
        : position(pos), rotation(rot), scale(scale), animationTime(0.0f) {}

    void setPosition(const Vec3& pos) { position = pos; }
    void setRotation(const Vec3& rot) { rotation = rot; }
    void setScale(const Vec3& sc) { scale = sc; }
    
    const Vec3& getPosition() const { return position; }
    const Vec3& getRotation() const { return rotation; }
    const Vec3& getScale() const { return scale; }

    void update(float deltaTime) { animationTime += deltaTime; }
    float getAnimationTime() const { return animationTime; }
    Mat4 getModelMatrix() const;
};

class Scene {
private:
    wgpu::Device* device;
    std::unique_ptr<ShaderManager> shaderManager;
    std::unique_ptr<Camera> camera;
    std::vector<std::unique_ptr<CubeObject>> cubeObjects;

    // Rendering resources for cubes
    wgpu::RenderPipeline cubePipeline;
    wgpu::Buffer cubeVertexBuffer;
    wgpu::Buffer cubeIndexBuffer;
    wgpu::Buffer cubeUniformBuffer;
    wgpu::BindGroup cubeBindGroup;
    wgpu::BindGroupLayout cubeBindGroupLayout;

    // Dynamic uniform buffer management
    static constexpr uint32_t MAX_CUBES = 100;
    static constexpr uint32_t UNIFORM_ALIGNMENT = 256; // WebGPU alignment requirement
    uint32_t alignedUniformSize;

    static constexpr uint32_t CUBE_VERTEX_COUNT = 8;
    static constexpr uint32_t CUBE_INDEX_COUNT = 36;

public:
    Scene(wgpu::Device* dev);
    ~Scene() = default;

    bool initialize();
    void update(float deltaTime);
    void render(wgpu::RenderPassEncoder& renderPass);

    // Camera management
    Camera* getCamera() { return camera.get(); }
    void setCamera(std::unique_ptr<Camera> cam) { camera = std::move(cam); }

    // Object management
    void addCube(const Vec3& position = Vec3(0, 0, 0), 
                 const Vec3& rotation = Vec3(0, 0, 0), 
                 const Vec3& scale = Vec3(1, 1, 1));
    void removeAllCubes();
    size_t getCubeCount() const { return cubeObjects.size(); }

private:
    bool createCubeRenderingResources();
    bool createCubeBuffers();
    bool createCubeBindGroupLayout();
    bool createCubePipeline();
    void updateCubeUniforms(const CubeObject& cube, size_t cubeIndex);

    static std::array<float, CUBE_VERTEX_COUNT * 3> getCubeVertices();
    static std::array<uint32_t, CUBE_INDEX_COUNT> getCubeIndices();
};

} // namespace rendering
} // namespace rs_engine