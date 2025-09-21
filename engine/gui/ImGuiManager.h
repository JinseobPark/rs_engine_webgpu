#pragma once

#include <webgpu/webgpu_cpp.h>
#ifdef __EMSCRIPTEN__
#include <GLFW/glfw3.h>
#else
#include <GLFW/glfw3.h>
#endif

namespace rs_engine {
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

    void showDebugWindow();
    void showSceneDebugger();
    void showPerformanceMetrics();
    void showWebGPUInfo();
    void showMemoryUsage();

    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    GLFWwindow* m_window = nullptr;
    wgpu::Device m_device;

    // Debug state
    bool m_showDebugWindow = true;
    bool m_showSceneDebugger = false;
    bool m_showPerformanceMetrics = false;
    bool m_showWebGPUInfo = false;
    bool m_showMemoryUsage = false;
    bool m_showDemo = false;

    // Performance tracking
    float m_frameTime = 0.0f;
    float m_fps = 0.0f;
    int m_frameCount = 0;
    double m_lastTime = 0.0;
};

} // namespace gui
} // namespace rs_engine