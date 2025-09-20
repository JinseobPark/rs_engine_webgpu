#pragma once

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

struct Mat4 {
    float data[16];

    Mat4() {
        // Identity matrix
        for (int i = 0; i < 16; i++) data[i] = 0.0f;
        data[0] = data[5] = data[10] = data[15] = 1.0f;
    }

    static Mat4 perspective(float fov, float aspect, float near, float far);
    static Mat4 lookAt(const std::array<float, 3>& eye, const std::array<float, 3>& center, const std::array<float, 3>& up);
    static Mat4 rotationY(float angle);
    static Mat4 multiply(const Mat4& a, const Mat4& b);
};

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