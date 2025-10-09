#pragma once

#include <webgpu/webgpu_cpp.h>
#ifdef __EMSCRIPTEN__
#include <GLFW/glfw3.h>
#else
#include <GLFW/glfw3.h>
#endif

namespace rs_engine {
// Forward declarations
class RenderSystem;

namespace gui {

class ImGuiManager {
public:
    ImGuiManager();
    ~ImGuiManager();

    bool initialize(GLFWwindow* window, wgpu::Device& device, wgpu::TextureFormat swapChainFormat);
    bool initializeForWeb(wgpu::Device& device, wgpu::TextureFormat swapChainFormat);
    void shutdown();

    void newFrame();
    void render(wgpu::RenderPassEncoder& renderPass);
    void onWindowResize(int width, int height);

    void showDebugWindow();
    void showSceneDebugger();
    void showPerformanceMetrics();
    void showWebGPUInfo();
    void showMemoryUsage();
    void setupDockspace();  // Setup main docking space
    void resetDockingLayout();  // Reset docking layout to default
    void saveDockingLayout();   // Save current layout (placeholder)
    void loadDockingLayout();   // Load saved layout (placeholder)
    
    // Game Engine GUI panels
    void showHierarchy();       // Scene hierarchy window
    void showInspector();       // Object inspector window
    void showProject();         // Project browser window
    void showConsole();         // Console/log window
    void showViewportControls(); // Viewport control panel (compact)
    void showAssets();          // Asset browser window
    void showSceneViewport();   // 3D Scene viewport window

    bool isInitialized() const { return m_initialized; }
    
    // Set render system reference
    void setRenderSystem(RenderSystem* renderSys) { m_renderSystem = renderSys; }

private:
    bool m_initialized = false;
    GLFWwindow* m_window = nullptr;
    wgpu::Device m_device;
    RenderSystem* m_renderSystem = nullptr;

    // Debug state
    bool m_showDebugWindow = true;
    bool m_showSceneDebugger = false;
    bool m_showPerformanceMetrics = false;
    bool m_showWebGPUInfo = false;
    bool m_showMemoryUsage = false;
    bool m_dockspaceEnabled = true;
    bool m_showMenuBar = true;
    
    // Game Engine GUI panels
    bool m_showHierarchy = true;
    bool m_showInspector = true;
    bool m_showProject = true;
    bool m_showConsole = true;
    bool m_showViewport = true;
    bool m_showAssets = true;
    bool m_showSceneViewport = true;

    // Performance tracking
    float m_frameTime = 0.0f;
    float m_fps = 0.0f;
    int m_frameCount = 0;
    double m_lastTime = 0.0;

    // Scene texture binding for ImGui
    void* m_sceneTextureID = nullptr;
    
    // Selected object tracking
    enum class SelectedObjectType {
        None,
        Camera,
        Light,
        Cube
    };
    SelectedObjectType m_selectedObjectType = SelectedObjectType::None;
    int m_selectedObjectIndex = -1;  // For cubes: 0, 1, 2
    
    // Viewport state (for picking)
    struct ViewportState {
        float width = 0.0f;
        float height = 0.0f;
        float posX = 0.0f;      // Viewport position in window
        float posY = 0.0f;
        bool isHovered = false;  // Is mouse over viewport?
    };
    ViewportState m_viewportState;

public:
    const ViewportState& getViewportState() const { return m_viewportState; }
};

} // namespace gui
} // namespace rs_engine