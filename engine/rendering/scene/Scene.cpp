#include "Scene.h"
#include <iostream>
#include <cstring>

namespace rs_engine {


namespace rendering {

Mat4 CubeObject::getModelMatrix() const {
    // Create transformation matrices using the new Mat4 library
    Mat4 translationMat = Mat4::translation(position);
    Mat4 rotationMat = Mat4::rotationY(animationTime);
    Mat4 scaleMat = Mat4::scale(scale);

    // Combine transformations: translation * rotation * scale
    return translationMat * rotationMat * scaleMat;
}

Scene::Scene(wgpu::Device* dev) : device(dev) {
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
    std::cout << "🎯 Scene::initialize() started..." << std::endl;

    if (!createCubeRenderingResources()) {
        std::cerr << "❌ Scene: Failed to create cube rendering resources" << std::endl;
        return false;
    }

    std::cout << "✅ Scene::initialize() completed successfully!" << std::endl;
    return true;
}

void Scene::update(float deltaTime) {
    // Update all cube objects
    for (auto& cube : cubeObjects) {
        cube->update(deltaTime);
    }
}

void Scene::render(wgpu::RenderPassEncoder& renderPass) {
    if (cubeObjects.empty()) {
        return; // Nothing to render
    }

    // Set cube pipeline
    renderPass.SetPipeline(cubePipeline);
    renderPass.SetVertexBuffer(0, cubeVertexBuffer);
    renderPass.SetIndexBuffer(cubeIndexBuffer, wgpu::IndexFormat::Uint32);

    // Render each cube with dynamic offset
    for (size_t i = 0; i < cubeObjects.size() && i < MAX_CUBES; ++i) {
        updateCubeUniforms(*cubeObjects[i], i);

        // Set bind group with dynamic offset
        uint32_t offset = static_cast<uint32_t>(i * alignedUniformSize);
        renderPass.SetBindGroup(0, cubeBindGroup, 1, &offset);

        renderPass.DrawIndexed(CUBE_INDEX_COUNT);
    }
}

void Scene::addCube(const Vec3& position, const Vec3& rotation, const Vec3& scale) {
    auto cube = std::make_unique<CubeObject>(position, rotation, scale);
    cubeObjects.push_back(std::move(cube));
    std::cout << "✅ Added cube to scene. Total cubes: " << cubeObjects.size() << std::endl;
}

void Scene::removeAllCubes() {
    cubeObjects.clear();
    std::cout << "🧹 Removed all cubes from scene" << std::endl;
}

bool Scene::createCubeRenderingResources() {
    if (!createCubeBuffers()) {
        std::cerr << "❌ Scene: createCubeBuffers() failed" << std::endl;
        return false;
    }
    std::cout << "✅ Scene: createCubeBuffers() succeeded" << std::endl;

    if (!createCubeBindGroupLayout()) {
        std::cerr << "❌ Scene: createCubeBindGroupLayout() failed" << std::endl;
        return false;
    }
    std::cout << "✅ Scene: createCubeBindGroupLayout() succeeded" << std::endl;

    if (!createCubePipeline()) {
        std::cerr << "❌ Scene: createCubePipeline() failed" << std::endl;
        return false;
    }
    std::cout << "✅ Scene: createCubePipeline() succeeded" << std::endl;

    return true;
}

bool Scene::createCubeBuffers() {
    // Create vertex buffer
    auto vertices = getCubeVertices();
    wgpu::BufferDescriptor vertexBufferDesc{};
    vertexBufferDesc.size = vertices.size() * sizeof(float);
    vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    cubeVertexBuffer = device->CreateBuffer(&vertexBufferDesc);

    if (!cubeVertexBuffer) {
        std::cerr << "Failed to create cube vertex buffer" << std::endl;
        return false;
    }

    device->GetQueue().WriteBuffer(cubeVertexBuffer, 0, vertices.data(), vertexBufferDesc.size);

    // Create index buffer
    auto indices = getCubeIndices();
    wgpu::BufferDescriptor indexBufferDesc{};
    indexBufferDesc.size = indices.size() * sizeof(uint32_t);
    indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
    cubeIndexBuffer = device->CreateBuffer(&indexBufferDesc);

    if (!cubeIndexBuffer) {
        std::cerr << "Failed to create cube index buffer" << std::endl;
        return false;
    }

    device->GetQueue().WriteBuffer(cubeIndexBuffer, 0, indices.data(), indexBufferDesc.size);

    // Calculate aligned uniform size for dynamic offsets
    alignedUniformSize = ((sizeof(CubeUniforms) + UNIFORM_ALIGNMENT - 1) / UNIFORM_ALIGNMENT) * UNIFORM_ALIGNMENT;

    // Create uniform buffer with space for multiple cubes
    wgpu::BufferDescriptor uniformBufferDesc{};
    uniformBufferDesc.size = alignedUniformSize * MAX_CUBES;
    uniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    cubeUniformBuffer = device->CreateBuffer(&uniformBufferDesc);

    if (!cubeUniformBuffer) {
        std::cerr << "Failed to create cube uniform buffer" << std::endl;
        return false;
    }

    return true;
}

bool Scene::createCubeBindGroupLayout() {
    wgpu::BindGroupLayoutEntry layoutEntry{};
    layoutEntry.binding = 0;
    layoutEntry.visibility = wgpu::ShaderStage::Vertex;
    layoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
    layoutEntry.buffer.hasDynamicOffset = true;
    layoutEntry.buffer.minBindingSize = sizeof(CubeUniforms);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &layoutEntry;

    cubeBindGroupLayout = device->CreateBindGroupLayout(&layoutDesc);
    if (!cubeBindGroupLayout) {
        std::cerr << "Failed to create cube bind group layout" << std::endl;
        return false;
    }

    // Create bind group
    wgpu::BindGroupEntry bindGroupEntry{};
    bindGroupEntry.binding = 0;
    bindGroupEntry.buffer = cubeUniformBuffer;
    bindGroupEntry.offset = 0;
    bindGroupEntry.size = alignedUniformSize;

    wgpu::BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = cubeBindGroupLayout;
    bindGroupDesc.entryCount = 1;
    bindGroupDesc.entries = &bindGroupEntry;

    cubeBindGroup = device->CreateBindGroup(&bindGroupDesc);
    if (!cubeBindGroup) {
        std::cerr << "Failed to create cube bind group" << std::endl;
        return false;
    }

    return true;
}

bool Scene::createCubePipeline() {
    wgpu::ShaderModule vertexShader = shaderManager->loadShader("render/cube_vertex.wgsl");
    wgpu::ShaderModule fragmentShader = shaderManager->loadShader("render/cube_fragment.wgsl");

    if (!vertexShader || !fragmentShader) {
        std::cerr << "Failed to load cube shaders" << std::endl;
        return false;
    }

    wgpu::RenderPipelineDescriptor pipelineDesc{};

    // Vertex stage
    pipelineDesc.vertex.module = vertexShader;
    pipelineDesc.vertex.entryPoint = "vs_main";

    // Vertex buffer layout
    wgpu::VertexAttribute positionAttr{};
    positionAttr.format = wgpu::VertexFormat::Float32x3;
    positionAttr.offset = 0;
    positionAttr.shaderLocation = 0;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = 3 * sizeof(float);
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
    layoutDesc.bindGroupLayouts = &cubeBindGroupLayout;

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

    cubePipeline = device->CreateRenderPipeline(&pipelineDesc);
    if (!cubePipeline) {
        std::cerr << "Failed to create cube render pipeline" << std::endl;
        return false;
    }

    return true;
}

void Scene::updateCubeUniforms(const CubeObject& cube, size_t cubeIndex) {
    CubeUniforms uniforms;
    uniforms.viewProj = camera->getViewProjectionMatrix();
    uniforms.model = cube.getModelMatrix();
    uniforms.time = cube.getAnimationTime();

    // Write to the specific offset for this cube
    uint32_t offset = static_cast<uint32_t>(cubeIndex * alignedUniformSize);
    device->GetQueue().WriteBuffer(cubeUniformBuffer, offset, &uniforms, sizeof(CubeUniforms));
}

std::array<float, Scene::CUBE_VERTEX_COUNT * 3> Scene::getCubeVertices() {
    // Make cube larger for better visibility
    float size = 1.0f;
    return {{
        // Front face
        -size, -size,  size,
         size, -size,  size,
         size,  size,  size,
        -size,  size,  size,
        // Back face
        -size, -size, -size,
         size, -size, -size,
         size,  size, -size,
        -size,  size, -size,
    }};
}

std::array<uint32_t, Scene::CUBE_INDEX_COUNT> Scene::getCubeIndices() {
    return {{
        // Front face (counter-clockwise from outside)
        0, 1, 2,  2, 3, 0,
        // Back face (counter-clockwise from outside) 
        5, 4, 7,  7, 6, 5,
        // Left face
        4, 0, 3,  3, 7, 4,
        // Right face  
        1, 5, 6,  6, 2, 1,
        // Top face
        3, 2, 6,  6, 7, 3,
        // Bottom face
        0, 4, 5,  5, 1, 0,
    }};
}

} // namespace rendering
} // namespace rs_engine