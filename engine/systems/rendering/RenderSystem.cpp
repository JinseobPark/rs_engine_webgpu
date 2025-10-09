#include "RenderSystem.h"
#include "../../core/Engine.h"
#include "../../core/Config.h"
#include "../../core/math/Ray.h"
#include "../application/ApplicationSystem.h"
#include "../input/InputSystem.h"
#include "../resource/ResourceSystem.h"
#include <iostream>
#include <cassert>
#include <limits>
#include <algorithm>
#include <vector>

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
    sceneDepthTextureView = nullptr;
    sceneDepthTexture = nullptr;
    sceneRenderTextureView = nullptr;
    sceneRenderTexture = nullptr;
#endif

    // 4. Release shared depth buffer
    depthTextureView = nullptr;
    depthTexture = nullptr;

    // 5. Finally shutdown scene
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

    // Set render system reference for GUI to access scene texture and input system
    guiManager->setRenderSystem(this);

    std::cout << "[SUCCESS] GUI initialized successfully!" << std::endl;
    return true;
}

bool RenderSystem::createSceneRenderTarget() {
    // Create color texture
    wgpu::TextureDescriptor colorTextureDesc = {};
    colorTextureDesc.dimension = wgpu::TextureDimension::e2D;
    colorTextureDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    colorTextureDesc.mipLevelCount = 1;
    colorTextureDesc.sampleCount = 1;
    colorTextureDesc.size = {sceneTextureWidth, sceneTextureHeight, 1};
    colorTextureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    colorTextureDesc.viewFormatCount = 0;
    colorTextureDesc.viewFormats = nullptr;

    sceneRenderTexture = appSystem->getDevice().CreateTexture(&colorTextureDesc);
    if (!sceneRenderTexture) {
        std::cerr << "[ERROR] Failed to create scene render texture" << std::endl;
        return false;
    }

    wgpu::TextureViewDescriptor colorViewDesc = {};
    colorViewDesc.format = colorTextureDesc.format;
    colorViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    colorViewDesc.baseMipLevel = 0;
    colorViewDesc.mipLevelCount = 1;
    colorViewDesc.baseArrayLayer = 0;
    colorViewDesc.arrayLayerCount = 1;

    sceneRenderTextureView = sceneRenderTexture.CreateView(&colorViewDesc);
    if (!sceneRenderTextureView) {
        std::cerr << "[ERROR] Failed to create scene render texture view" << std::endl;
        return false;
    }
    
    // Create depth texture
    wgpu::TextureDescriptor depthTextureDesc = {};
    depthTextureDesc.dimension = wgpu::TextureDimension::e2D;
    depthTextureDesc.format = wgpu::TextureFormat::Depth24Plus;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {sceneTextureWidth, sceneTextureHeight, 1};
    depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    
    sceneDepthTexture = appSystem->getDevice().CreateTexture(&depthTextureDesc);
    if (!sceneDepthTexture) {
        std::cerr << "[ERROR] Failed to create scene depth texture" << std::endl;
        return false;
    }
    
    wgpu::TextureViewDescriptor depthViewDesc = {};
    depthViewDesc.format = depthTextureDesc.format;
    depthViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    depthViewDesc.baseMipLevel = 0;
    depthViewDesc.mipLevelCount = 1;
    depthViewDesc.baseArrayLayer = 0;
    depthViewDesc.arrayLayerCount = 1;
    
    sceneDepthTextureView = sceneDepthTexture.CreateView(&depthViewDesc);
    if (!sceneDepthTextureView) {
        std::cerr << "[ERROR] Failed to create scene depth texture view" << std::endl;
        return false;
    }

    std::cout << "[SUCCESS] Scene render target created with depth buffer (" << sceneTextureWidth 
              << "x" << sceneTextureHeight << ")" << std::endl;
    return true;
}

void RenderSystem::renderToTexture() {
    if (!sceneRenderTextureView || !sceneDepthTextureView) {
        return;
    }

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = appSystem->getDevice().CreateCommandEncoder(&encoderDesc);

    // Color attachment
    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = sceneRenderTextureView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.2, 0.3, 0.3, 1.0};
    
    // Depth attachment
    wgpu::RenderPassDepthStencilAttachment depthAttachment = {};
    depthAttachment.view = sceneDepthTextureView;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthClearValue = 1.0f;
    depthAttachment.depthReadOnly = false;

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = &depthAttachment;

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

bool RenderSystem::ensureDepthTexture(uint32_t width, uint32_t height) {
    // Check if we need to recreate depth texture
    if (depthTexture && lastDepthTextureWidth == width && lastDepthTextureHeight == height) {
        return true; // Already have correct size
    }

    // Release old depth texture
    depthTextureView = nullptr;
    depthTexture = nullptr;

    // Create new depth texture
    wgpu::TextureDescriptor depthTextureDesc = {};
    depthTextureDesc.dimension = wgpu::TextureDimension::e2D;
    depthTextureDesc.format = wgpu::TextureFormat::Depth24Plus;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {width, height, 1};
    depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;

    depthTexture = appSystem->getDevice().CreateTexture(&depthTextureDesc);
    if (!depthTexture) {
        std::cerr << "[ERROR] Failed to create depth texture" << std::endl;
        return false;
    }

    wgpu::TextureViewDescriptor depthViewDesc = {};
    depthViewDesc.format = depthTextureDesc.format;
    depthViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    depthViewDesc.baseMipLevel = 0;
    depthViewDesc.mipLevelCount = 1;
    depthViewDesc.baseArrayLayer = 0;
    depthViewDesc.arrayLayerCount = 1;

    depthTextureView = depthTexture.CreateView(&depthViewDesc);
    if (!depthTextureView) {
        std::cerr << "[ERROR] Failed to create depth texture view" << std::endl;
        return false;
    }

    lastDepthTextureWidth = width;
    lastDepthTextureHeight = height;

    return true;
}

void RenderSystem::render() {
#ifdef __EMSCRIPTEN__
    // ===== WEB: Direct scene rendering (no ImGui, use HTML UI) =====
    wgpu::SurfaceTexture surfaceTexture;
    appSystem->getSurface().GetCurrentTexture(&surfaceTexture);

    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
        return;
    }

    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    // Ensure depth buffer matches surface size
    uint32_t width = appSystem->getWindowWidth();
    uint32_t height = appSystem->getWindowHeight();
    if (!ensureDepthTexture(width, height)) {
        std::cerr << "[ERROR] Failed to create depth texture for web render" << std::endl;
        return;
    }

    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder encoder = appSystem->getDevice().CreateCommandEncoder(&encoderDesc);

    wgpu::RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0};

    // Depth attachment
    wgpu::RenderPassDepthStencilAttachment depthAttachment = {};
    depthAttachment.view = depthTextureView;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthAttachment.depthClearValue = 1.0f;
    depthAttachment.depthReadOnly = false;

    wgpu::RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = &depthAttachment;

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

// ========== Object Picking ==========

rendering::SceneObject* RenderSystem::pickObject(float screenX, float screenY) {
    if (!scene) return nullptr;
    
    // Create ray from screen coordinates
    Ray ray = createRayFromScreen(screenX, screenY);
    
    // Always use AABB + Triangle intersection (precise picking)
    auto config = EngineConfig::getPickingConfig();
    
    // Phase 1: AABB filtering - collect candidates
    struct Candidate {
        rendering::SceneObject* object;
        float aabbDistance;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(config.maxCandidates);
    
    for (const auto& [name, objectPtr] : scene->getAllObjects()) {
        if (!objectPtr || !objectPtr->hasModel()) continue;

        Vec3 min, max;
        objectPtr->getWorldBounds(min, max);

        float tMin, tMax;
        if (ray.intersectAABB(min, max, tMin, tMax) && tMin >= 0) {
            std::cout << "[Picking] " << name << " AABB hit at distance " << tMin
                      << " (bounds: min=" << min.x << "," << min.y << "," << min.z
                      << " max=" << max.x << "," << max.y << "," << max.z << ")" << std::endl;
            candidates.push_back({objectPtr.get(), tMin});
        } else {
            std::cout << "[Picking] " << name << " AABB miss (bounds: min="
                      << min.x << "," << min.y << "," << min.z
                      << " max=" << max.x << "," << max.y << "," << max.z << ")" << std::endl;
        }
    }
    
    if (candidates.empty()) {
        return nullptr;
    }
    
    // Sort candidates by distance (closest first)
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                  return a.aabbDistance < b.aabbDistance;
              });
    
    // Limit to top N candidates for performance
    if (candidates.size() > config.maxCandidates) {
        candidates.resize(config.maxCandidates);
    }
    
    // Phase 2: Precise triangle intersection test
    rendering::SceneObject* closestObject = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    
    for (const auto& candidate : candidates) {
        float t = intersectObjectTriangles(ray, candidate.object);
        
        if (t >= 0 && t < closestDistance) {
            closestDistance = t;
            closestObject = candidate.object;
        }
    }
    
    // If no triangle intersection found, fall back to closest AABB candidate
    if (!closestObject && !candidates.empty()) {
        std::cout << "[Picking] No triangle hit, using closest AABB candidate" << std::endl;
        closestObject = candidates[0].object;
    }
    
    if (closestObject) {
        std::cout << "[Picking] Precise hit at distance: " << closestDistance << std::endl;
    }
    
    return closestObject;
}

float RenderSystem::intersectObjectTriangles(const Ray& ray, rendering::SceneObject* obj) {
    if (!obj || !obj->hasModel()) {
        return -1.0f;
    }
    
    auto model = obj->getModel();
    if (!model) {
        return -1.0f;
    }
    
    Mat4 modelMatrix = obj->getModelMatrix();
    
    float closestT = std::numeric_limits<float>::max();
    bool hitFound = false;
    
    // Test each mesh in the model
    for (const auto& mesh : model->getMeshes()) {
        if (!mesh) continue;
        
        const auto& vertices = mesh->getVertices();
        const auto& indices = mesh->getIndices();
        
        // Test each triangle
        for (size_t i = 0; i < indices.size(); i += 3) {
            // Get triangle vertices in model space
            Vec3 v0 = vertices[indices[i]].position;
            Vec3 v1 = vertices[indices[i + 1]].position;
            Vec3 v2 = vertices[indices[i + 2]].position;
            
            // Transform to world space
            Vec3 w0 = modelMatrix.transformPoint(v0);
            Vec3 w1 = modelMatrix.transformPoint(v1);
            Vec3 w2 = modelMatrix.transformPoint(v2);
            
            // Test intersection
            float t;
            if (ray.intersectTriangle(w0, w1, w2, t)) {
                if (t < closestT) {
                    closestT = t;
                    hitFound = true;
                }
            }
        }
    }
    
    return hitFound ? closestT : -1.0f;
}

rendering::SceneObject* RenderSystem::getSelectedObject() {
    return scene ? scene->getSelectedObject() : nullptr;
}

void RenderSystem::setSelectedObject(rendering::SceneObject* object) {
    if (scene) {
        scene->setSelectedObject(object);
    }
}

void RenderSystem::clearSelection() {
    if (scene) {
        scene->clearSelection();
    }
}

Ray RenderSystem::createRayFromScreen(float screenX, float screenY) const {
    if (!appSystem || !scene) {
        return Ray();
    }
    
    // Get camera
    auto* camera = scene->getCamera();
    if (!camera) {
        return Ray();
    }
    
    float width, height;
    float viewportOffsetX = 0.0f;
    float viewportOffsetY = 0.0f;
    
#ifdef __EMSCRIPTEN__
    // Web: Use full window size (no ImGui viewport)
    width = static_cast<float>(appSystem->getWindowWidth());
    height = static_cast<float>(appSystem->getWindowHeight());
#else
    // Native: Use ImGui viewport size and offset
    if (guiManager) {
        const auto& viewport = guiManager->getViewportState();
        width = viewport.width;
        height = viewport.height;
        viewportOffsetX = viewport.posX;
        viewportOffsetY = viewport.posY;
    } else {
        // Fallback to window size
        width = static_cast<float>(appSystem->getWindowWidth());
        height = static_cast<float>(appSystem->getWindowHeight());
    }
#endif
    
    if (width == 0.0f || height == 0.0f) {
        return Ray();
    }
    
    // CRITICAL: Always update aspect ratio before using projection matrix
    float currentAspect = width / height;
    const_cast<rendering::Camera*>(camera)->setAspectRatio(currentAspect);
    
    // Convert screen coordinates to viewport-relative coordinates
    float viewportX = screenX - viewportOffsetX;
    float viewportY = screenY - viewportOffsetY;
    
    // Convert viewport coordinates to NDC (-1 to 1)
    // Note: Screen space origin is top-left, NDC origin is center
    float ndcX = (2.0f * viewportX) / width - 1.0f;
    float ndcY = 1.0f - (2.0f * viewportY) / height;  // Y is inverted
    
    // Get fresh view and projection matrices (after aspect ratio update)
    Mat4 view = camera->getViewMatrix();
    Mat4 proj = camera->getProjectionMatrix();
    
    // Combine and invert to get world space transform
    Mat4 viewProj = proj * view;
    Mat4 invViewProj = viewProj.inverse();
    
    // Create points in NDC space at near and far planes
    // Near plane: -1 (OpenGL convention) or 0 (D3D convention)
    // Using -1 for OpenGL-style NDC
    Vec3 nearPoint = invViewProj.transformPoint(Vec3(ndcX, ndcY, -1.0f));
    Vec3 farPoint = invViewProj.transformPoint(Vec3(ndcX, ndcY, 1.0f));
    
    // Create ray from camera origin through the point
    Vec3 rayOrigin = nearPoint;
    Vec3 rayDirection = (farPoint - nearPoint).normalized();
    
    return Ray(rayOrigin, rayDirection);
}

} // namespace rs_engine
