#include "Application.h"
#include <cassert>

namespace rs_engine {

bool Application::createRenderPipeline() {
    // Vertex shader (WGSL)
    const char* vertexShaderSource = R"(
@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 3>(
        vec2f( 0.0,  0.5),
        vec2f(-0.5, -0.5),
        vec2f( 0.5, -0.5)
    );
    return vec4f(pos[vertexIndex], 0.0, 1.0);
}
)";

    // Fragment shader (WGSL)
    const char* fragmentShaderSource = R"(
@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(1.0, 0.5, 0.2, 1.0);
}
)";

    // Create shader modules
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.code = vertexShaderSource;

    wgpu::ShaderModuleDescriptor vertexShaderDesc = {};
    vertexShaderDesc.nextInChain = &wgslDesc;
    wgpu::ShaderModule vertexShader = device.CreateShaderModule(&vertexShaderDesc);

    wgslDesc.code = fragmentShaderSource;
    wgpu::ShaderModuleDescriptor fragmentShaderDesc = {};
    fragmentShaderDesc.nextInChain = &wgslDesc;
    wgpu::ShaderModule fragmentShader = device.CreateShaderModule(&fragmentShaderDesc);

    // Create render pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc = {};
    
    // Vertex stage
    pipelineDesc.vertex.module = vertexShader;
    pipelineDesc.vertex.entryPoint = "vs_main";

    // Fragment stage
    wgpu::FragmentState fragmentState = {};
    fragmentState.module = fragmentShader;
    fragmentState.entryPoint = "fs_main";

    // Color target
    wgpu::ColorTargetState colorTarget = {};
    colorTarget.format = wgpu::TextureFormat::BGRA8Unorm;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;

    // Primitive state
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
    pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

    // Multisample state
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    pipeline = device.CreateRenderPipeline(&pipelineDesc);
    if (!pipeline) {
        std::cerr << "Failed to create render pipeline" << std::endl;
        return false;
    }

    return true;
}

void Application::configureSurface() {
    wgpu::SurfaceConfiguration surfaceConfig = {};
    surfaceConfig.device = device;
    surfaceConfig.format = wgpu::TextureFormat::BGRA8Unorm;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
    surfaceConfig.width = windowWidth;
    surfaceConfig.height = windowHeight;
    surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
    
    surface.Configure(&surfaceConfig);
}

void Application::render() {
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);
    
#ifdef __EMSCRIPTEN__
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
        return;
    }
#else
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        return;
    }
#endif

    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.2, 0.3, 0.3, 1.0};

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
    renderPass.SetPipeline(pipeline);
    renderPass.Draw(3, 1, 0, 0);
    renderPass.End();

    wgpu::CommandBufferDescriptor cmdBufferDesc = {};
    wgpu::CommandBuffer commands = encoder.Finish(&cmdBufferDesc);
    device.GetQueue().Submit(1, &commands);

#ifndef __EMSCRIPTEN__
    surface.Present();
#endif
}

void Application::run() {
#ifdef __EMSCRIPTEN__
    // 웹에서는 emscripten_set_main_loop에서 처리
    // 여기서는 초기화만 완료
    isInitialized = true;
#else
    // 네이티브에서는 직접 루프 실행
    while (!shouldClose) {
        handleEvents();
        update(0.016f); // 60fps 가정
        draw();
    }
#endif
}

} // namespace rs_engine
