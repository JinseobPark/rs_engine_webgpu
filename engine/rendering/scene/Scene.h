#pragma once

#include "../../core/math/Mat4.h"
#include "../../core/math/Vec3.h"
#include "Camera.h"
#include "SceneObject.h"
#include "../ShaderManager.h"
#include "../../resource/ResourceManager.h"
#include <memory>
#include <vector>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
    #include <webgpu/webgpu.h>
    #include <webgpu/webgpu_cpp.h>
#else
    #include <dawn/webgpu_cpp.h>
#endif

namespace rs_engine {

// Uniforms for rendering
struct ObjectUniforms {
    Mat4 viewProj;
    Mat4 model;
    float time;
    float padding[3]; // Ensure 16-byte alignment
};

namespace rendering {

class Scene {
private:
    wgpu::Device* device;
    resource::ResourceManager* resourceManager;
    std::unique_ptr<ShaderManager> shaderManager;
    std::unique_ptr<Camera> camera;
    
    // Scene objects (name -> object)
    std::unordered_map<std::string, std::unique_ptr<SceneObject>> sceneObjects;

    // Rendering resources (TEMPORARY - will be replaced with proper renderer)
    wgpu::RenderPipeline renderPipeline;
    wgpu::Buffer uniformBuffer;
    wgpu::BindGroup bindGroup;
    wgpu::BindGroupLayout bindGroupLayout;

    // Dynamic uniform buffer management
    static constexpr uint32_t MAX_OBJECTS = 100;
    static constexpr uint32_t UNIFORM_ALIGNMENT = 256; // WebGPU alignment requirement
    uint32_t alignedUniformSize;

public:
    Scene(wgpu::Device* dev, resource::ResourceManager* resMgr);
    ~Scene() = default;

    bool initialize();
    void update(float deltaTime);
    void render(wgpu::RenderPassEncoder& renderPass);

    // Camera management
    Camera* getCamera() { return camera.get(); }
    void setCamera(std::unique_ptr<Camera> cam) { camera = std::move(cam); }

    // ========== Object Management ==========
    
    /**
     * @brief Create an empty scene object
     * @param name Unique object name
     * @return Pointer to created object (or nullptr if name exists)
     */
    SceneObject* createObject(const std::string& name);
    
    /**
     * @brief Add a mesh to an existing object
     * @param objectName Name of the object
     * @param meshHandle Resource handle of the mesh (from ResourceManager)
     * @return true if successful
     */
    bool addMeshToObject(const std::string& objectName, resource::ResourceHandle meshHandle);
    
    /**
     * @brief Get scene object by name
     */
    SceneObject* getObject(const std::string& name);
    
    /**
     * @brief Remove object from scene
     */
    void removeObject(const std::string& name);
    
    /**
     * @brief Remove all objects
     */
    void clearAllObjects();
    
    /**
     * @brief Get object count
     */
    size_t getObjectCount() const { return sceneObjects.size(); }

private:
    bool createRenderingResources();
    bool createUniformBuffer();
    bool createBindGroupLayout();
    bool createRenderPipeline();
    void updateObjectUniforms(const SceneObject& object, size_t objectIndex);
    
    void renderObject(wgpu::RenderPassEncoder& renderPass, 
                     const SceneObject& object, 
                     size_t objectIndex);
};

} // namespace rendering
} // namespace rs_engine