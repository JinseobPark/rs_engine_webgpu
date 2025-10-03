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

bool Application::initializeGUI() {
    std::cout << "🎯 Initializing GUI..." << std::endl;

    guiManager = std::make_unique<rs_engine::gui::ImGuiManager>();

    if (!guiManager->initialize(getWindow(), device, wgpu::TextureFormat::BGRA8Unorm)) {
        std::cerr << "❌ Failed to initialize GUI" << std::endl;
        return false;
    }

    // Set application reference so ImGui can access render targets
    guiManager->setApplication(this);

    std::cout << "✅ GUI initialized successfully!" << std::endl;
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

bool Application::createSceneRenderTarget() {
    // Create render target texture for the scene viewport
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.size = {sceneTextureWidth, sceneTextureHeight, 1};
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;

    sceneRenderTexture = device.CreateTexture(&textureDesc);
    if (!sceneRenderTexture) {
        std::cerr << "Failed to create scene render texture" << std::endl;
        return false;
    }

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = textureDesc.format;
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;

    sceneRenderTextureView = sceneRenderTexture.CreateView(&viewDesc);
    if (!sceneRenderTextureView) {
        std::cerr << "Failed to create scene render texture view" << std::endl;
        return false;
    }

    std::cout << "✅ Scene render target created (" << sceneTextureWidth << "x" << sceneTextureHeight << ")" << std::endl;
    return true;
}

void Application::renderToTexture() {
    if (!sceneRenderTextureView) {
        return;
    }

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = sceneRenderTextureView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.2, 0.3, 0.3, 1.0}; // Same clear color as main render

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);

    // Render only the 3D scene to the texture (no GUI)
    if (scene) {
        scene->render(renderPass);
    }

    renderPass.End();

    wgpu::CommandBufferDescriptor cmdBufferDesc = {};
    wgpu::CommandBuffer commands = encoder.Finish(&cmdBufferDesc);
    device.GetQueue().Submit(1, &commands);
}

void Application::render() {
#ifdef __EMSCRIPTEN__
    // ===== WEB: Direct scene rendering (no ImGui, use HTML UI) =====
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);

    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
        return;
    }

    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0}; // Dark background

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);

    // ✅ Web: Render scene directly to canvas (no ImGui)
    if (scene) {
        scene->render(renderPass);
    }

    renderPass.End();

    wgpu::CommandBufferDescriptor cmdBufferDesc = {};
    wgpu::CommandBuffer commands = encoder.Finish(&cmdBufferDesc);
    device.GetQueue().Submit(1, &commands);
    
    // Web doesn't need Present() - browser handles it automatically
#else
    // ===== NATIVE: Render to texture for ImGui viewport, then render GUI =====
    // First render the 3D scene to our offscreen texture
    renderToTexture();

    // Then render the main surface with GUI
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);

    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        return;
    }

    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0}; // Darker background for GUI

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);

    // Native: Render GUI (including Scene Viewport showing the 3D texture)
    if (guiManager && guiManager->isInitialized()) {
        guiManager->newFrame();
        guiManager->render(renderPass);
    }

    renderPass.End();

    wgpu::CommandBufferDescriptor cmdBufferDesc = {};
    wgpu::CommandBuffer commands = encoder.Finish(&cmdBufferDesc);
    device.GetQueue().Submit(1, &commands);

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
