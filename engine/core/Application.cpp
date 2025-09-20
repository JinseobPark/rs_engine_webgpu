#include "Application.h"
#include <cassert>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace rs_engine {

bool Application::initializeRenderer() {
    // Kept for backward compatibility, now just calls initializeScene
    return initializeScene();
}

bool Application::initializeScene() {
    std::cout << "🎯 Initializing Scene..." << std::endl;

    scene = std::make_unique<rs_engine::rendering::Scene>(&device);

    if (!scene->initialize()) {
        std::cerr << "❌ Failed to initialize scene" << std::endl;
        return false;
    }

    // Add a default cube to the scene
    scene->addCube(Vec3(2, 0, 0));
    scene->addCube(Vec3(0, 3, 0));
    scene->addCube(Vec3(-2, 0, 0));

    std::cout << "✅ Scene initialized successfully!" << std::endl;
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

    if (scene) {
        scene->render(renderPass);
    }

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
    // 웹에서는 emscripten_set_main_loop로 루프 시작
    isInitialized = true;
    // WebApplication 전용 renderLoop 함수를 여기서 호출하도록 변경 필요
    // 이 부분은 WebApplication에서 오버라이드하거나 가상 함수로 처리
    emscripten_set_main_loop_arg([](void* userData) {
        Application* app = static_cast<Application*>(userData);
        if (app->isInitialized && !app->shouldClose) {
            app->handleEvents();
            float deltaTime = 0.016f; // 60fps
            app->update(deltaTime);
            if (app->scene) {
                app->scene->update(deltaTime);
            }
            app->draw();
        }
    }, this, 60, 1);
#else
    // 네이티브에서는 직접 루프 실행
    while (!shouldClose) {
        handleEvents();
        float deltaTime = 0.016f; // 60fps 가정
        update(deltaTime);
        if (scene) {
            scene->update(deltaTime);
        }
        draw();
    }
#endif
}

} // namespace rs_engine
