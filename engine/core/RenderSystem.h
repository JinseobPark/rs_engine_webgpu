#pragma once

#include "IEngineSystem.h"
#include "../rendering/scene/Scene.h"
#include "../gui/ImGuiManager.h"
#include <memory>

#ifdef __EMSCRIPTEN__
    #include <webgpu/webgpu_cpp.h>
#else
    #include <dawn/webgpu_cpp.h>
#endif

namespace rs_engine {

// Forward declaration
class ApplicationSystem;

/**
 * @brief Render System - Scene rendering and GUI
 * 
 * Responsibilities:
 * - Scene graph management
 * - Camera management
 * - Material and shader management
 * - GUI rendering (ImGui on Native, HTML on Web)
 * - Render target management
 * 
 * Platform Support: 90% shared
 * - Scene rendering: 100% shared
 * - GUI: Native only (ImGui), Web uses HTML
 */
class RenderSystem : public IEngineSystem {
private:
    ApplicationSystem* appSystem = nullptr;
    
    std::unique_ptr<rendering::Scene> scene;
    
#ifndef __EMSCRIPTEN__
    std::unique_ptr<gui::ImGuiManager> guiManager;
    
    // Render target for ImGui viewport
    wgpu::Texture sceneRenderTexture = nullptr;
    wgpu::TextureView sceneRenderTextureView = nullptr;
    uint32_t sceneTextureWidth = 800;
    uint32_t sceneTextureHeight = 600;
#endif

public:
    RenderSystem() = default;
    virtual ~RenderSystem() = default;

    // IEngineSystem interface
    bool initialize(Engine* engineRef) override;
    void onStart() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    const char* getName() const override { return "Render"; }
    int getPriority() const override { return 100; }

    // Scene access
    rendering::Scene* getScene() { return scene.get(); }
    
#ifndef __EMSCRIPTEN__
    gui::ImGuiManager* getGUI() { return guiManager.get(); }
    wgpu::TextureView getSceneTextureView() const { return sceneRenderTextureView; }
    uint32_t getSceneTextureWidth() const { return sceneTextureWidth; }
    uint32_t getSceneTextureHeight() const { return sceneTextureHeight; }
#endif

private:
    bool initializeScene();
    
#ifndef __EMSCRIPTEN__
    bool initializeGUI();
    bool createSceneRenderTarget();
    void renderToTexture();
#endif
    
    void render();
};

} // namespace rs_engine
