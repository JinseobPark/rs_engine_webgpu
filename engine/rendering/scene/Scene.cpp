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
    
    // Render bounding box for selected object
    if (selectedObject && selectedObject->hasModel()) {
        renderBoundingBox(renderPass, *selectedObject);
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
    
    if (!createBoundingBoxPipeline()) {
        std::cerr << "[ERROR] Scene: createBoundingBoxPipeline() failed" << std::endl;
        return false;
    }
    std::cout << "[SUCCESS] Scene: createBoundingBoxPipeline() succeeded" << std::endl;
    
    if (!createBoundingBoxGeometry()) {
        std::cerr << "[ERROR] Scene: createBoundingBoxGeometry() failed" << std::endl;
        return false;
    }
    std::cout << "[SUCCESS] Scene: createBoundingBoxGeometry() succeeded" << std::endl;

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
    
    // Depth/stencil state
    wgpu::DepthStencilState depthStencilState{};
    depthStencilState.format = wgpu::TextureFormat::Depth24Plus;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    pipelineDesc.depthStencil = &depthStencilState;

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

// ========== Selection Management ==========

void Scene::setSelectedObject(SceneObject* object) {
    // Deselect previous object
    if (selectedObject) {
        selectedObject->setSelected(false);
    }
    
    // Select new object
    selectedObject = object;
    if (selectedObject) {
        selectedObject->setSelected(true);
    }
}

// ========== Bounding Box Rendering ==========

bool Scene::createBoundingBoxPipeline() {
    // Load line shaders
    wgpu::ShaderModule vertexShaderModule = shaderManager->loadShader("render/line_vertex.wgsl");
    wgpu::ShaderModule fragmentShaderModule = shaderManager->loadShader("render/line_fragment.wgsl");
    
    if (!vertexShaderModule || !fragmentShaderModule) {
        std::cerr << "[ERROR] Failed to load line shaders" << std::endl;
        return false;
    }
    
    // Vertex buffer layout - only position needed for lines
    wgpu::VertexAttribute vertexAttribute{};
    vertexAttribute.format = wgpu::VertexFormat::Float32x3;
    vertexAttribute.offset = 0;
    vertexAttribute.shaderLocation = 0;
    
    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = sizeof(float) * 3; // x, y, z
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
    vertexBufferLayout.attributeCount = 1;
    vertexBufferLayout.attributes = &vertexAttribute;
    
    // Pipeline layout
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = &bindGroupLayout; // Reuse same layout
    wgpu::PipelineLayout pipelineLayout = device->CreatePipelineLayout(&layoutDesc);
    
    // Create render pipeline for lines
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.layout = pipelineLayout;
    
    // Vertex stage
    pipelineDesc.vertex.module = vertexShaderModule;
    pipelineDesc.vertex.entryPoint = "main";
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;
    
    // Fragment stage
    wgpu::FragmentState fragmentState{};
    fragmentState.module = fragmentShaderModule;
    fragmentState.entryPoint = "main";
    
    wgpu::ColorTargetState colorTarget{};
    colorTarget.format = wgpu::TextureFormat::BGRA8Unorm;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;
    
    // Enable blending for transparency
    wgpu::BlendState blendState{};
    blendState.color.operation = wgpu::BlendOperation::Add;
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.alpha.operation = wgpu::BlendOperation::Add;
    blendState.alpha.srcFactor = wgpu::BlendFactor::One;
    blendState.alpha.dstFactor = wgpu::BlendFactor::Zero;
    colorTarget.blend = &blendState;
    
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    
    // Primitive state - LINE LIST topology
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::LineList;
    pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
    pipelineDesc.primitive.cullMode = wgpu::CullMode::None; // Don't cull lines
    
    // Depth/stencil state
    wgpu::DepthStencilState depthStencilState{};
    depthStencilState.format = wgpu::TextureFormat::Depth24Plus;
    depthStencilState.depthWriteEnabled = false; // Don't write to depth buffer
    depthStencilState.depthCompare = wgpu::CompareFunction::Less; // Still test against depth
    pipelineDesc.depthStencil = &depthStencilState;
    
    // Multisample state
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    
    boundingBoxPipeline = device->CreateRenderPipeline(&pipelineDesc);
    if (!boundingBoxPipeline) {
        std::cerr << "[ERROR] Failed to create bounding box pipeline" << std::endl;
        return false;
    }
    
    return true;
}

bool Scene::createBoundingBoxGeometry() {
    // Create vertices for unit cube (will be transformed by object's bounds)
    // 8 vertices of a unit cube centered at origin
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  // 0: left  bottom back
         0.5f, -0.5f, -0.5f,  // 1: right bottom back
         0.5f,  0.5f, -0.5f,  // 2: right top    back
        -0.5f,  0.5f, -0.5f,  // 3: left  top    back
        -0.5f, -0.5f,  0.5f,  // 4: left  bottom front
         0.5f, -0.5f,  0.5f,  // 5: right bottom front
         0.5f,  0.5f,  0.5f,  // 6: right top    front
        -0.5f,  0.5f,  0.5f   // 7: left  top    front
    };
    
    // Create indices for 12 edges (24 indices for line list)
    uint32_t indices[] = {
        // Back face
        0, 1,  1, 2,  2, 3,  3, 0,
        // Front face
        4, 5,  5, 6,  6, 7,  7, 4,
        // Connecting edges
        0, 4,  1, 5,  2, 6,  3, 7
    };
    
    boundingBoxIndexCount = sizeof(indices) / sizeof(uint32_t);
    
    // Create vertex buffer
    wgpu::BufferDescriptor vertexBufferDesc{};
    vertexBufferDesc.size = sizeof(vertices);
    vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    boundingBoxVertexBuffer = device->CreateBuffer(&vertexBufferDesc);
    
    if (!boundingBoxVertexBuffer) {
        std::cerr << "[ERROR] Failed to create bounding box vertex buffer" << std::endl;
        return false;
    }
    
    device->GetQueue().WriteBuffer(boundingBoxVertexBuffer, 0, vertices, sizeof(vertices));
    
    // Create index buffer
    wgpu::BufferDescriptor indexBufferDesc{};
    indexBufferDesc.size = sizeof(indices);
    indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
    boundingBoxIndexBuffer = device->CreateBuffer(&indexBufferDesc);
    
    if (!boundingBoxIndexBuffer) {
        std::cerr << "[ERROR] Failed to create bounding box index buffer" << std::endl;
        return false;
    }
    
    device->GetQueue().WriteBuffer(boundingBoxIndexBuffer, 0, indices, sizeof(indices));
    
    return true;
}

void Scene::renderBoundingBox(wgpu::RenderPassEncoder& renderPass, const SceneObject& object) {
    if (!boundingBoxPipeline || !boundingBoxVertexBuffer || !boundingBoxIndexBuffer) {
        return;
    }

    // Get world bounds
    Vec3 minBound, maxBound;
    object.getWorldBounds(minBound, maxBound);

    // Calculate center and size
    Vec3 center = (minBound + maxBound) * 0.5f;
    Vec3 size = maxBound - minBound;

    // Create transform matrix for bounding box
    Mat4 boxTransform = Mat4::translation(center) * Mat4::scale(size);

    // Update uniforms for bounding box
    ObjectUniforms uniforms;
    uniforms.viewProj = camera->getViewProjectionMatrix();
    uniforms.model = boxTransform;
    uniforms.time = 0.0f;

    // Use the last available slot (MAX_OBJECTS - 1) to avoid overwriting object uniforms
    // This ensures we don't corrupt any object's uniform data
    uint32_t boundingBoxSlot = MAX_OBJECTS - 1;
    uint32_t boundingBoxOffset = static_cast<uint32_t>(boundingBoxSlot * alignedUniformSize);
    device->GetQueue().WriteBuffer(uniformBuffer, boundingBoxOffset, &uniforms, sizeof(ObjectUniforms));

    // Set pipeline and bind group
    renderPass.SetPipeline(boundingBoxPipeline);
    renderPass.SetBindGroup(0, bindGroup, 1, &boundingBoxOffset);

    // Set vertex and index buffers
    renderPass.SetVertexBuffer(0, boundingBoxVertexBuffer);
    renderPass.SetIndexBuffer(boundingBoxIndexBuffer, wgpu::IndexFormat::Uint32);

    // Draw lines
    renderPass.DrawIndexed(boundingBoxIndexCount, 1, 0, 0, 0);
}

} // namespace rendering
} // namespace rs_engine