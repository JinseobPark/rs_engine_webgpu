#include "ImGuiManager.h"
#include <imgui.h>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include "../platform/NativeApplication.h"
#endif

namespace rs_engine {
namespace gui {

ImGuiManager::ImGuiManager() = default;

ImGuiManager::~ImGuiManager() {
    if (m_initialized) {
        shutdown();
    }
}

bool ImGuiManager::initialize(GLFWwindow* window, wgpu::Device& device, wgpu::TextureFormat swapChainFormat) {
    if (m_initialized) {
        std::cerr << "ImGuiManager already initialized!" << std::endl;
        return false;
    }

#ifdef __EMSCRIPTEN__
    return initializeForWeb(device, swapChainFormat);
#else
    m_window = window;
    m_device = device;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Note: Docking may not be available in all ImGui versions
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOther(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        return false;
    }

    ImGui_ImplWGPU_InitInfo init_info = {};
    init_info.Device = device.Get();
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(swapChainFormat);
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;

    if (!ImGui_ImplWGPU_Init(&init_info)) {
        std::cerr << "Failed to initialize ImGui WebGPU backend" << std::endl;
        ImGui_ImplGlfw_Shutdown();
        return false;
    }

    m_initialized = true;
    m_lastTime = glfwGetTime();

    std::cout << "✅ ImGui initialized successfully!" << std::endl;
    return true;
#endif
}

bool ImGuiManager::initializeForWeb(wgpu::Device& device, wgpu::TextureFormat swapChainFormat) {
    if (m_initialized) {
        std::cerr << "ImGuiManager already initialized!" << std::endl;
        return false;
    }

    m_device = device;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Note: Docking may not be available in all ImGui versions
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // For web, we need to properly initialize the font atlas
    // Build the default font atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Set display size for web
#ifdef __EMSCRIPTEN__
    double canvas_w, canvas_h;
    emscripten_get_element_css_size("#canvas", &canvas_w, &canvas_h);
    io.DisplaySize = ImVec2((float)canvas_w, (float)canvas_h);
#else
    io.DisplaySize = ImVec2(800.0f, 600.0f); // Default size for non-web
#endif
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    m_initialized = true;
#ifdef __EMSCRIPTEN__
    m_lastTime = emscripten_get_now() / 1000.0;
#else
    m_lastTime = 0.0; // This should not be called for native builds
#endif

    std::cout << "✅ ImGui initialized successfully for web (minimal mode)!" << std::endl;
    return true;
}

void ImGuiManager::shutdown() {
    if (!m_initialized) {
        return;
    }

#ifdef __EMSCRIPTEN__
    // For web, minimal shutdown
    ImGui::DestroyContext();
#else
    ImGui_ImplWGPU_Shutdown();
    // Check if GLFW is still initialized before calling ImGui_ImplGlfw_Shutdown
    // This prevents the "GLFW library is not initialized" error during cleanup
    if (NativeApplication::isGLFWInitialized()) {
        ImGui_ImplGlfw_Shutdown();
    }
    ImGui::DestroyContext();
#endif

    m_initialized = false;
    std::cout << "🔄 ImGui shutdown complete" << std::endl;
}

void ImGuiManager::newFrame() {
    if (!m_initialized) {
        return;
    }

    // Update performance metrics
#ifdef __EMSCRIPTEN__
    double currentTime = emscripten_get_now() / 1000.0;
#else
    double currentTime = glfwGetTime();
#endif
    m_frameTime = static_cast<float>(currentTime - m_lastTime);
    m_lastTime = currentTime;
    m_frameCount++;

    if (m_frameCount % 60 == 0) { // Update FPS every 60 frames
        m_fps = 1.0f / m_frameTime;
    }

    // Start the Dear ImGui frame
#ifdef __EMSCRIPTEN__
    // For web, skip ImGui to avoid complications - use HTML UI instead
    return;
#else
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
#endif

    // Show debug windows
    if (m_showDebugWindow) {
        showDebugWindow();
    }

    if (m_showSceneDebugger) {
        showSceneDebugger();
    }

    if (m_showPerformanceMetrics) {
        showPerformanceMetrics();
    }

    if (m_showWebGPUInfo) {
        showWebGPUInfo();
    }

    if (m_showMemoryUsage) {
        showMemoryUsage();
    }

    if (m_showDemo) {
        ImGui::ShowDemoWindow(&m_showDemo);
    }
}

void ImGuiManager::render(wgpu::RenderPassEncoder& renderPass) {
    if (!m_initialized) {
        return;
    }

#ifdef __EMSCRIPTEN__
    // For web, skip ImGui rendering - HTML UI provides debugging interface
    return;
#else
    // Render Dear ImGui for native builds
    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass.Get());
#endif
}

void ImGuiManager::showDebugWindow() {
    ImGui::Begin("Debug Controls", &m_showDebugWindow);

    ImGui::Text("RS Engine WebGPU Debug Panel");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Windows")) {
        ImGui::Checkbox("Scene Debugger", &m_showSceneDebugger);
        ImGui::Checkbox("Performance Metrics", &m_showPerformanceMetrics);
        ImGui::Checkbox("WebGPU Info", &m_showWebGPUInfo);
        ImGui::Checkbox("Memory Usage", &m_showMemoryUsage);
        ImGui::Checkbox("ImGui Demo", &m_showDemo);
    }

    if (ImGui::CollapsingHeader("System Info")) {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

#ifdef __EMSCRIPTEN__
        // For web, get canvas size
        double canvas_w, canvas_h;
        emscripten_get_element_css_size("#canvas", &canvas_w, &canvas_h);
        ImGui::Text("Canvas Size: %.0fx%.0f", canvas_w, canvas_h);
#else
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        ImGui::Text("Display Size: %dx%d", display_w, display_h);
#endif
    }

    ImGui::End();
}

void ImGuiManager::showSceneDebugger() {
    ImGui::Begin("Scene Debugger", &m_showSceneDebugger);

    ImGui::Text("Scene debugging tools will be added here");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Text("Camera controls will be added here");
        // TODO: Add camera position, rotation, etc.
    }

    if (ImGui::CollapsingHeader("Objects")) {
        ImGui::Text("Scene objects list will be added here");
        // TODO: Add scene objects list and properties
    }

    if (ImGui::CollapsingHeader("Lighting")) {
        ImGui::Text("Lighting controls will be added here");
        // TODO: Add lighting controls
    }

    ImGui::End();
}

void ImGuiManager::showPerformanceMetrics() {
    ImGui::Begin("Performance Metrics", &m_showPerformanceMetrics);

    ImGui::Text("Frame Time: %.3f ms", m_frameTime * 1000.0f);
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::Text("Frame Count: %d", m_frameCount);

    ImGui::Separator();

    // Simple frame time graph
    static float frameTimeHistory[100] = {0};
    static int frameTimeIndex = 0;
    frameTimeHistory[frameTimeIndex] = m_frameTime * 1000.0f;
    frameTimeIndex = (frameTimeIndex + 1) % 100;

    ImGui::PlotLines("Frame Time (ms)", frameTimeHistory, 100, frameTimeIndex, nullptr, 0.0f, 50.0f, ImVec2(0, 80));

    ImGui::End();
}

void ImGuiManager::showWebGPUInfo() {
    ImGui::Begin("WebGPU Info", &m_showWebGPUInfo);

    ImGui::Text("WebGPU Device Information");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Device Details")) {
        // Device info would typically require WebGPU API calls
        ImGui::Text("Device: WebGPU Device");
        ImGui::Text("Backend: %s",
#ifdef __EMSCRIPTEN__
            "WebGPU (Browser)"
#else
            "Dawn (Native)"
#endif
        );

        ImGui::Text("Platform: %s",
#ifdef __EMSCRIPTEN__
            "Web/Emscripten"
#elif defined(__APPLE__)
            "macOS"
#elif defined(_WIN32)
            "Windows"
#else
            "Linux"
#endif
        );
    }

    if (ImGui::CollapsingHeader("Supported Features")) {
        ImGui::Text("• Vertex Shaders");
        ImGui::Text("• Fragment Shaders");
        ImGui::Text("• Compute Shaders");
        ImGui::Text("• Render Targets");
        ImGui::Text("• Depth Testing");
    }

    if (ImGui::CollapsingHeader("Limits")) {
        ImGui::Text("Max Texture Size: 8192x8192");
        ImGui::Text("Max Vertex Attributes: 16");
        ImGui::Text("Max Uniform Buffer Size: 64KB");
        ImGui::Text("Max Compute Work Groups: 256x256x64");
    }

    ImGui::End();
}

void ImGuiManager::showMemoryUsage() {
    ImGui::Begin("Memory Usage", &m_showMemoryUsage);

    ImGui::Text("Memory Usage Information");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("GPU Memory")) {
        ImGui::Text("Vertex Buffers: ~%.1f KB", 256.0f); // Placeholder values
        ImGui::Text("Index Buffers: ~%.1f KB", 64.0f);
        ImGui::Text("Uniform Buffers: ~%.1f KB", 32.0f);
        ImGui::Text("Textures: ~%.1f MB", 2.5f);

        ImGui::Separator();
        ImGui::Text("Total GPU Memory: ~%.1f MB", 2.85f);
    }

    if (ImGui::CollapsingHeader("System Memory")) {
#ifdef __EMSCRIPTEN__
        size_t used_memory = 0;
        size_t total_memory = 0;

        // Get memory info from emscripten
        EM_ASM({
            var performance = window.performance || {};
            var memory = performance.memory || {};
            setValue($0, memory.usedJSHeapSize || 0, 'i32');
            setValue($1, memory.totalJSHeapSize || 0, 'i32');
        }, &used_memory, &total_memory);

        if (total_memory > 0) {
            ImGui::Text("Used Heap: %.1f MB", used_memory / (1024.0f * 1024.0f));
            ImGui::Text("Total Heap: %.1f MB", total_memory / (1024.0f * 1024.0f));

            float usage_percent = (float)used_memory / (float)total_memory * 100.0f;
            ImGui::ProgressBar(usage_percent / 100.0f, ImVec2(0, 0),
                             (std::string("Memory: ") + std::to_string((int)usage_percent) + "%").c_str());
        } else {
            ImGui::Text("Memory info not available");
        }
#else
        ImGui::Text("System memory info not implemented for native");
#endif
    }

    if (ImGui::CollapsingHeader("Engine Objects")) {
        ImGui::Text("Scene Objects: 3"); // From the scene setup
        ImGui::Text("Shaders Loaded: 2"); // Vertex + Fragment
        ImGui::Text("Buffers Created: 6"); // Vertex, Index, Uniform per cube
        ImGui::Text("Active Textures: 0"); // No textures yet
    }

    ImGui::End();
}

} // namespace gui
} // namespace rs_engine