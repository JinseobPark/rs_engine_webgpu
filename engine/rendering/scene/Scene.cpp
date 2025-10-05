#include "Scene.h"
#include <iostream>
#include <cstring>

namespace rs_engine {

namespace rendering {

Scene::Scene(wgpu::Device* dev, resource::ResourceManager* resMgr) 
    : device(dev), resourceManager(resMgr) {
    shaderManager = std::make_unique<ShaderManager>(device, "shaders/");
    
    // Create default camera
    camera = std::make_unique<Camera>(
        45.0f * M_PI / 180.0f,  // 45 degree FOV
        800.0f / 600.0f,        // 4:3 aspect ratio
        0.1f,                   // near plane
        100.0f                  // far plane
    );
    
    // Set up default camera position
    camera->lookAt(Vec3(0, 0, 20), Vec3(0, 0, 0), Vec3(0, 1, 0));
}

bool Scene::initialize() {
    std::cout << "[INFO] Scene::initialize() started..." << std::endl;

    if (!createRenderingResources()) {
        return false;
    }

    std::cout << "[SUCCESS] Scene::initialize() completed successfully!" << std::endl;
    return true;
}

void Scene::update(float deltaTime) {
    // Update all scene objects
    for (auto& [name, object] : sceneObjects) {
        object->update(deltaTime);
    }
}

void Scene::render(wgpu::RenderPassEncoder& renderPass) {
    if (sceneObjects.empty()) {
        return;
    }

    // Set render pipeline
    renderPass.SetPipeline(renderPipeline);

    // Render each object
    size_t objectIndex = 0;
    for (const auto& [name, object] : sceneObjects) {
        if (objectIndex >= MAX_OBJECTS) break;
        if (!object->getVisible() || !object->hasModel()) continue;
        
        renderObject(renderPass, *object, objectIndex);
        objectIndex++;
    }
}

// ========== Object Management ==========

SceneObject* Scene::createObject(const std::string& name) {
    // Check if object already exists
    if (sceneObjects.find(name) != sceneObjects.end()) {
        std::cerr << "[ERROR] Scene object '" << name << "' already exists" << std::endl;
        return nullptr;
    }
    
    auto object = std::make_unique<SceneObject>(name);
    SceneObject* objectPtr = object.get();
    sceneObjects[name] = std::move(object);
    
    std::cout << "[SUCCESS] Created scene object '" << name << "'" << std::endl;
    return objectPtr;
}

bool Scene::addMeshToObject(const std::string& objectName, resource::ResourceHandle meshHandle) {
    auto it = sceneObjects.find(objectName);
    if (it == sceneObjects.end()) {
        std::cerr << "[ERROR] Scene object '" << objectName << "' not found" << std::endl;
        return false;
    }
    
    if (!resourceManager) {
        std::cerr << "[ERROR] ResourceManager not available" << std::endl;
        return false;
    }
    
    // Get the mesh from ResourceManager
    auto mesh = resourceManager->getMesh(meshHandle);
    if (!mesh) {
        std::cerr << "[ERROR] Mesh with handle " << meshHandle << " not found" << std::endl;
        return false;
    }
    
    // Create a model with this mesh
    auto model = std::make_shared<resource::Model>(objectName + "_Model");
    model->addMesh(mesh);
    
    // Set the model on the object
    it->second->setModel(model);
    
    std::cout << "[SUCCESS] Added mesh to object '" << objectName << "'" << std::endl;
    return true;
}

SceneObject* Scene::getObject(const std::string& name) {
    auto it = sceneObjects.find(name);
    return (it != sceneObjects.end()) ? it->second.get() : nullptr;
}

void Scene::removeObject(const std::string& name) {
    auto it = sceneObjects.find(name);
    if (it != sceneObjects.end()) {
        sceneObjects.erase(it);
        std::cout << "[INFO] Removed object '" << name << "' from scene" << std::endl;
    }
}

void Scene::clearAllObjects() {
    sceneObjects.clear();
    std::cout << "[INFO] Cleared all objects from scene" << std::endl;
}



// ========== Rendering Resource Creation ==========

bool Scene::createRenderingResources() {
    if (!createUniformBuffer()) {
        std::cerr << "[ERROR] Scene: createUniformBuffer() failed" << std::endl;
        return false;
    }
    std::cout << "[SUCCESS] Scene: createUniformBuffer() succeeded" << std::endl;

    if (!createBindGroupLayout()) {
        std::cerr << "[ERROR] Scene: createBindGroupLayout() failed" << std::endl;
        return false;
    }
    std::cout << "[SUCCESS] Scene: createBindGroupLayout() succeeded" << std::endl;

    if (!createRenderPipeline()) {
        std::cerr << "[ERROR] Scene: createRenderPipeline() failed" << std::endl;
        return false;
    }
    std::cout << "[SUCCESS] Scene: createRenderPipeline() succeeded" << std::endl;

    return true;
}

bool Scene::createUniformBuffer() {
    // Calculate aligned uniform size for dynamic offsets
    alignedUniformSize = ((sizeof(ObjectUniforms) + UNIFORM_ALIGNMENT - 1) / UNIFORM_ALIGNMENT) * UNIFORM_ALIGNMENT;

    // Create uniform buffer with space for multiple objects
    wgpu::BufferDescriptor uniformBufferDesc{};
    uniformBufferDesc.size = alignedUniformSize * MAX_OBJECTS;
    uniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    uniformBuffer = device->CreateBuffer(&uniformBufferDesc);

    if (!uniformBuffer) {
        std::cerr << "[ERROR] Failed to create uniform buffer" << std::endl;
        return false;
    }

    return true;
}

bool Scene::createBindGroupLayout() {
    wgpu::BindGroupLayoutEntry layoutEntry{};
    layoutEntry.binding = 0;
    layoutEntry.visibility = wgpu::ShaderStage::Vertex;
    layoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
    layoutEntry.buffer.hasDynamicOffset = true;
    layoutEntry.buffer.minBindingSize = sizeof(ObjectUniforms);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &layoutEntry;

    bindGroupLayout = device->CreateBindGroupLayout(&layoutDesc);
    if (!bindGroupLayout) {
        std::cerr << "[ERROR] Failed to create bind group layout" << std::endl;
        return false;
    }

    // Create bind group
    wgpu::BindGroupEntry bindGroupEntry{};
    bindGroupEntry.binding = 0;
    bindGroupEntry.buffer = uniformBuffer;
    bindGroupEntry.offset = 0;
    bindGroupEntry.size = alignedUniformSize;

    wgpu::BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = 1;
    bindGroupDesc.entries = &bindGroupEntry;

    bindGroup = device->CreateBindGroup(&bindGroupDesc);
    if (!bindGroup) {
        std::cerr << "[ERROR] Failed to create bind group" << std::endl;
        return false;
    }

    return true;
}

bool Scene::createRenderPipeline() {
    wgpu::ShaderModule vertexShader = shaderManager->loadShader("render/cube_vertex.wgsl");
    wgpu::ShaderModule fragmentShader = shaderManager->loadShader("render/cube_fragment.wgsl");

    if (!vertexShader || !fragmentShader) {
        std::cerr << "[ERROR] Failed to load shaders" << std::endl;
        return false;
    }

    wgpu::RenderPipelineDescriptor pipelineDesc{};

    // Vertex stage
    pipelineDesc.vertex.module = vertexShader;
    pipelineDesc.vertex.entryPoint = "vs_main";

    // Vertex buffer layout
    // Vertex struct: position(12) + normal(12) + texCoord(12) + color(12) = 48 bytes
    wgpu::VertexAttribute positionAttr{};
    positionAttr.format = wgpu::VertexFormat::Float32x3;
    positionAttr.offset = 0;  // Position is at offset 0
    positionAttr.shaderLocation = 0;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = 12 * sizeof(float);  // 48 bytes: Vec3(pos) + Vec3(norm) + Vec3(tex) + Vec3(col)
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
    vertexBufferLayout.attributeCount = 1;
    vertexBufferLayout.attributes = &positionAttr;

    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;

    // Fragment stage
    wgpu::FragmentState fragmentState{};
    fragmentState.module = fragmentShader;
    fragmentState.entryPoint = "fs_main";

    wgpu::ColorTargetState colorTarget{};
    colorTarget.format = wgpu::TextureFormat::BGRA8Unorm;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;

    // Pipeline layout
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = &bindGroupLayout;

    wgpu::PipelineLayout pipelineLayout = device->CreatePipelineLayout(&layoutDesc);
    pipelineDesc.layout = pipelineLayout;

    // Primitive state
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
    pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

    // Multisample state
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    renderPipeline = device->CreateRenderPipeline(&pipelineDesc);
    if (!renderPipeline) {
        std::cerr << "[ERROR] Failed to create render pipeline" << std::endl;
        return false;
    }

    return true;
}

// ========== Rendering ==========

void Scene::updateObjectUniforms(const SceneObject& object, size_t objectIndex) {
    ObjectUniforms uniforms;
    uniforms.viewProj = camera->getViewProjectionMatrix();
    uniforms.model = object.getModelMatrix();
    uniforms.time = object.getAnimationTime();

    // Write to the specific offset for this object
    uint32_t offset = static_cast<uint32_t>(objectIndex * alignedUniformSize);
    device->GetQueue().WriteBuffer(uniformBuffer, offset, &uniforms, sizeof(ObjectUniforms));
}

void Scene::renderObject(wgpu::RenderPassEncoder& renderPass, 
                        const SceneObject& object, 
                        size_t objectIndex) {
    auto model = object.getModel();
    if (!model) return;
    
    // Update uniforms
    updateObjectUniforms(object, objectIndex);
    
    // Set bind group with dynamic offset
    uint32_t dynamicOffset = static_cast<uint32_t>(objectIndex * alignedUniformSize);
    renderPass.SetBindGroup(0, bindGroup, 1, &dynamicOffset);
    
    // Render each mesh in the model
    const auto& meshes = model->getMeshes();
    for (const auto& mesh : meshes) {
        if (!mesh || !mesh->hasGPUResources()) continue;
        
        // Set vertex and index buffers
        renderPass.SetVertexBuffer(0, mesh->getVertexBuffer());
        renderPass.SetIndexBuffer(mesh->getIndexBuffer(), wgpu::IndexFormat::Uint32);
        
        // Draw
        renderPass.DrawIndexed(static_cast<uint32_t>(mesh->getIndexCount()), 1, 0, 0, 0);
    }
}

} // namespace rendering
} // namespace rs_engine