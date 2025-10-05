#include "RenderSystem.h"
#include "../../core/Engine.h"
#include "../application/ApplicationSystem.h"
#include "../input/InputSystem.h"
#include "../resource/ResourceSystem.h"
#include <iostream>
#include <cassert>

namespace rs_engine {

bool RenderSystem::initialize(Engine* engineRef) {
    if (!IEngineSystem::initialize(engineRef)) {
        return false;
    }

    std::cout << "[INFO] Initializing Render System..." << std::endl;

    // Get ApplicationSystem
    appSystem = engine->getSystem<ApplicationSystem>();
    if (!appSystem) {
        std::cerr << "[ERROR] ApplicationSystem not found" << std::endl;
        return false;
    }

    // Get InputSystem (optional, for camera control)
    inputSystem = engine->getSystem<InputSystem>();
    if (!inputSystem) {
        std::cout << "[WARNING] InputSystem not found - camera control will be disabled" << std::endl;
    }

    if (!initializeScene()) {
        std::cerr << "[ERROR] Failed to initialize scene" << std::endl;
        return false;
    }
    
    // Initialize camera controller in InputSystem
    if (scene && inputSystem && scene->getCamera()) {
        inputSystem->initializeCameraController(scene->getCamera());
    }

#ifndef __EMSCRIPTEN__
    if (!initializeGUI()) {
        std::cerr << "[ERROR] Failed to initialize GUI" << std::endl;
        return false;
    }

    if (!createSceneRenderTarget()) {
        std::cerr << "[ERROR] Failed to create scene render target" << std::endl;
        return false;
    }
#endif

    std::cout << "[SUCCESS] Render System initialized" << std::endl;
    return true;
}

void RenderSystem::onStart() {
    std::cout << "[Render] Started - Scene ready" << std::endl;
}

void RenderSystem::onUpdate(float deltaTime) {
    if (scene) {
        scene->update(deltaTime);
    }
    
    render();
}

void RenderSystem::onShutdown() {
    std::cout << "[Render] Shutting down..." << std::endl;
    
#ifndef __EMSCRIPTEN__
    // Shutdown in reverse order of initialization
    // 1. First explicitly shutdown GUI before destroying
    if (guiManager && guiManager->isInitialized()) {
        guiManager->shutdown();
    }
    
    // 2. Then destroy GUI manager
    guiManager.reset();
    
    // 3. Then release render targets
    sceneRenderTextureView = nullptr;
    sceneRenderTexture = nullptr;
#endif
    
    // 4. Finally shutdown scene
    if (scene) {
        scene.reset();
    }
    
    std::cout << "[Render] Shutdown complete" << std::endl;
}

bool RenderSystem::initializeScene() {
    std::cout << "[INFO] Initializing Scene..." << std::endl;

    // Get ResourceSystem
    auto* resourceSystem = engine->getSystem<ResourceSystem>();
    if (!resourceSystem) {
        std::cerr << "[ERROR] ResourceSystem not found" << std::endl;
        return false;
    }

    scene = std::make_unique<rendering::Scene>(&appSystem->getDevice(), 
                                                resourceSystem->getResourceManager());

    if (!scene->initialize()) {
        std::cerr << "[ERROR] Failed to initialize scene" << std::endl;
        return false;
    }

    std::cout << "[SUCCESS] Scene initialized successfully!" << std::endl;
    return true;
}

#ifndef __EMSCRIPTEN__
bool RenderSystem::initializeGUI() {
    std::cout << "[INFO] Initializing GUI..." << std::endl;

    guiManager = std::make_unique<gui::ImGuiManager>();

    if (!guiManager->initialize(appSystem->getWindow(), appSystem->getDevice(), 
                                 wgpu::TextureFormat::BGRA8Unorm)) {
        std::cerr << "[ERROR] Failed to initialize GUI" << std::endl;
        return false;
    }

    // Set render system reference for GUI to access scene texture
    guiManager->setRenderSystem(this);

    std::cout << "[SUCCESS] GUI initialized successfully!" << std::endl;
    return true;
}

bool RenderSystem::createSceneRenderTarget() {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.size = {sceneTextureWidth, sceneTextureHeight, 1};
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;

    sceneRenderTexture = appSystem->getDevice().CreateTexture(&textureDesc);
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

    std::cout << "[SUCCESS] Scene render target created (" << sceneTextureWidth 
              << "x" << sceneTextureHeight << ")" << std::endl;
    return true;
}

void RenderSystem::renderToTexture() {
    if (!sceneRenderTextureView) {
        return;
    }

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = appSystem->getDevice().CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = sceneRenderTextureView;
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
    appSystem->getDevice().GetQueue().Submit(1, &commands);
}
#endif

void RenderSystem::render() {
#ifdef __EMSCRIPTEN__
    // ===== WEB: Direct scene rendering (no ImGui, use HTML UI) =====
    wgpu::SurfaceTexture surfaceTexture;
    appSystem->getSurface().GetCurrentTexture(&surfaceTexture);

    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
        return;
    }

    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = appSystem->getDevice().CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0};

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
    appSystem->getDevice().GetQueue().Submit(1, &commands);
    
#else
    // ===== NATIVE: Render to texture for ImGui viewport, then render GUI =====
    renderToTexture();

    wgpu::SurfaceTexture surfaceTexture;
    appSystem->getSurface().GetCurrentTexture(&surfaceTexture);

    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        return;
    }

    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = appSystem->getDevice().CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0};

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);

    if (guiManager && guiManager->isInitialized()) {
        guiManager->newFrame();
        guiManager->render(renderPass);
    }

    renderPass.End();

    wgpu::CommandBufferDescriptor cmdBufferDesc = {};
    wgpu::CommandBuffer commands = encoder.Finish(&cmdBufferDesc);
    appSystem->getDevice().GetQueue().Submit(1, &commands);

    appSystem->getSurface().Present();
#endif
}

} // namespace rs_engine
