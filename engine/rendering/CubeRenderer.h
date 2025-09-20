#pragma once

#include "../core/math/Mat4.h"
#include "../core/math/Vec3.h"
#include "ShaderManager.h"
#include <array>
#include <cmath>

#ifdef __EMSCRIPTEN__
    #include <webgpu/webgpu.h>
    #include <webgpu/webgpu_cpp.h>
#else
    #include <dawn/webgpu_cpp.h>
#endif

namespace rs_engine {

struct CubeUniforms {
    Mat4 viewProj;
    Mat4 model;
    float time;
    float padding[3]; // Ensure 16-byte alignment
};

class CubeRenderer {
private:
    wgpu::Device* device;
    std::unique_ptr<ShaderManager> shaderManager;

    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    wgpu::Buffer indexBuffer;
    wgpu::Buffer uniformBuffer;
    wgpu::BindGroup bindGroup;
    wgpu::BindGroupLayout bindGroupLayout;

    CubeUniforms uniforms;
    float currentTime = 0.0f;

    static constexpr uint32_t VERTEX_COUNT = 8;
    static constexpr uint32_t INDEX_COUNT = 36;

public:
    CubeRenderer(wgpu::Device* dev);
    ~CubeRenderer() = default;

    bool initialize();
    void update(float deltaTime);
    void render(wgpu::RenderPassEncoder& renderPass);

private:
    bool createBuffers();
    bool createBindGroupLayout();
    bool createPipeline();
    void updateUniforms();

    static std::array<float, VERTEX_COUNT * 3> getCubeVertices();
    static std::array<uint32_t, INDEX_COUNT> getCubeIndices();
};

} // namespace rs_engine