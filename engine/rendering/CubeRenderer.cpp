#include "CubeRenderer.h"
#include <iostream>
#include <cstring>

namespace rs_engine {

// Mat4 implementation
Mat4 Mat4::perspective(float fov, float aspect, float near, float far) {
    Mat4 result;
    float tanHalfFov = tan(fov / 2.0f);

    result.data[0] = 1.0f / (aspect * tanHalfFov);
    result.data[5] = 1.0f / tanHalfFov;
    result.data[10] = -(far + near) / (far - near);
    result.data[11] = -1.0f;
    result.data[14] = -(2.0f * far * near) / (far - near);
    result.data[15] = 0.0f;

    return result;
}

Mat4 Mat4::lookAt(const std::array<float, 3>& eye, const std::array<float, 3>& center, const std::array<float, 3>& up) {
    Mat4 result;

    // Calculate forward vector
    float fx = center[0] - eye[0];
    float fy = center[1] - eye[1];
    float fz = center[2] - eye[2];
    float flen = sqrt(fx*fx + fy*fy + fz*fz);
    fx /= flen; fy /= flen; fz /= flen;

    // Calculate right vector
    float rx = fy * up[2] - fz * up[1];
    float ry = fz * up[0] - fx * up[2];
    float rz = fx * up[1] - fy * up[0];
    float rlen = sqrt(rx*rx + ry*ry + rz*rz);
    rx /= rlen; ry /= rlen; rz /= rlen;

    // Calculate up vector
    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;

    result.data[0] = rx; result.data[1] = ux; result.data[2] = -fx; result.data[3] = 0;
    result.data[4] = ry; result.data[5] = uy; result.data[6] = -fy; result.data[7] = 0;
    result.data[8] = rz; result.data[9] = uz; result.data[10] = -fz; result.data[11] = 0;
    result.data[12] = -(rx * eye[0] + ry * eye[1] + rz * eye[2]);
    result.data[13] = -(ux * eye[0] + uy * eye[1] + uz * eye[2]);
    result.data[14] = -(-fx * eye[0] + -fy * eye[1] + -fz * eye[2]);
    result.data[15] = 1;

    return result;
}

Mat4 Mat4::rotationY(float angle) {
    Mat4 result;
    float c = cos(angle);
    float s = sin(angle);

    result.data[0] = c;   result.data[2] = s;
    result.data[8] = -s;  result.data[10] = c;

    return result;
}

Mat4 Mat4::multiply(const Mat4& a, const Mat4& b) {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.data[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result.data[i * 4 + j] += a.data[i * 4 + k] * b.data[k * 4 + j];
            }
        }
    }
    return result;
}

// CubeRenderer implementation
CubeRenderer::CubeRenderer(wgpu::Device* dev) : device(dev) {
    shaderManager = std::make_unique<ShaderManager>(device, "shaders/");
}

std::array<float, CubeRenderer::VERTEX_COUNT * 3> CubeRenderer::getCubeVertices() {
    return {{
        // Front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // Back face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
    }};
}

std::array<uint32_t, CubeRenderer::INDEX_COUNT> CubeRenderer::getCubeIndices() {
    return {{
        // Front face
        0, 1, 2,  2, 3, 0,
        // Back face
        4, 6, 5,  6, 4, 7,
        // Left face
        4, 0, 3,  3, 7, 4,
        // Right face
        1, 5, 6,  6, 2, 1,
        // Top face
        3, 2, 6,  6, 7, 3,
        // Bottom face
        4, 5, 1,  1, 0, 4,
    }};
}

bool CubeRenderer::createBuffers() {
    // Create vertex buffer
    auto vertices = getCubeVertices();
    wgpu::BufferDescriptor vertexBufferDesc{};
    vertexBufferDesc.size = vertices.size() * sizeof(float);
    vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    vertexBuffer = device->CreateBuffer(&vertexBufferDesc);

    if (!vertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return false;
    }

    device->GetQueue().WriteBuffer(vertexBuffer, 0, vertices.data(), vertexBufferDesc.size);

    // Create index buffer
    auto indices = getCubeIndices();
    wgpu::BufferDescriptor indexBufferDesc{};
    indexBufferDesc.size = indices.size() * sizeof(uint32_t);
    indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
    indexBuffer = device->CreateBuffer(&indexBufferDesc);

    if (!indexBuffer) {
        std::cerr << "Failed to create index buffer" << std::endl;
        return false;
    }

    device->GetQueue().WriteBuffer(indexBuffer, 0, indices.data(), indexBufferDesc.size);

    // Create uniform buffer
    wgpu::BufferDescriptor uniformBufferDesc{};
    uniformBufferDesc.size = sizeof(CubeUniforms);
    uniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    uniformBuffer = device->CreateBuffer(&uniformBufferDesc);

    if (!uniformBuffer) {
        std::cerr << "Failed to create uniform buffer" << std::endl;
        return false;
    }

    return true;
}

bool CubeRenderer::createBindGroupLayout() {
    wgpu::BindGroupLayoutEntry layoutEntry{};
    layoutEntry.binding = 0;
    layoutEntry.visibility = wgpu::ShaderStage::Vertex;
    layoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
    layoutEntry.buffer.hasDynamicOffset = false;
    layoutEntry.buffer.minBindingSize = sizeof(CubeUniforms);

    wgpu::BindGroupLayoutDescriptor layoutDesc{};
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &layoutEntry;

    bindGroupLayout = device->CreateBindGroupLayout(&layoutDesc);
    if (!bindGroupLayout) {
        std::cerr << "Failed to create bind group layout" << std::endl;
        return false;
    }

    // Create bind group
    wgpu::BindGroupEntry bindGroupEntry{};
    bindGroupEntry.binding = 0;
    bindGroupEntry.buffer = uniformBuffer;
    bindGroupEntry.offset = 0;
    bindGroupEntry.size = sizeof(CubeUniforms);

    wgpu::BindGroupDescriptor bindGroupDesc{};
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = 1;
    bindGroupDesc.entries = &bindGroupEntry;

    bindGroup = device->CreateBindGroup(&bindGroupDesc);
    if (!bindGroup) {
        std::cerr << "Failed to create bind group" << std::endl;
        return false;
    }

    return true;
}

bool CubeRenderer::createPipeline() {
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

    pipeline = device->CreateRenderPipeline(&pipelineDesc);
    if (!pipeline) {
        std::cerr << "Failed to create cube render pipeline" << std::endl;
        return false;
    }

    return true;
}

bool CubeRenderer::initialize() {
    std::cout << "🎯 CubeRenderer::initialize() started..." << std::endl;

    if (!createBuffers()) {
        std::cerr << "❌ CubeRenderer: createBuffers() failed" << std::endl;
        return false;
    }
    std::cout << "✅ CubeRenderer: createBuffers() succeeded" << std::endl;

    if (!createBindGroupLayout()) {
        std::cerr << "❌ CubeRenderer: createBindGroupLayout() failed" << std::endl;
        return false;
    }
    std::cout << "✅ CubeRenderer: createBindGroupLayout() succeeded" << std::endl;

    if (!createPipeline()) {
        std::cerr << "❌ CubeRenderer: createPipeline() failed" << std::endl;
        return false;
    }
    std::cout << "✅ CubeRenderer: createPipeline() succeeded" << std::endl;

    // Initialize projection matrix
    uniforms.viewProj = Mat4::multiply(
        Mat4::perspective(45.0f * M_PI / 180.0f, 800.0f / 600.0f, 0.1f, 100.0f),
        Mat4::lookAt({0, 0, 3}, {0, 0, 0}, {0, 1, 0})
    );

    std::cout << "✅ CubeRenderer::initialize() completed successfully!" << std::endl;
    return true;
}

void CubeRenderer::updateUniforms() {
    uniforms.model = Mat4::rotationY(currentTime);
    uniforms.time = currentTime;

    device->GetQueue().WriteBuffer(uniformBuffer, 0, &uniforms, sizeof(CubeUniforms));
}

void CubeRenderer::update(float deltaTime) {
    currentTime += deltaTime;
    updateUniforms();
}

void CubeRenderer::render(wgpu::RenderPassEncoder& renderPass) {
    renderPass.SetPipeline(pipeline);
    renderPass.SetBindGroup(0, bindGroup);
    renderPass.SetVertexBuffer(0, vertexBuffer);
    renderPass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
    renderPass.DrawIndexed(INDEX_COUNT);
}

} // namespace rs_engine