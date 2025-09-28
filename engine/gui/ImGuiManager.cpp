#include "ImGuiManager.h"
#include "../core/Application.h"
#include <imgui.h>
#include <imgui_internal.h>  // Required for DockBuilder API
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Customize style for better docking experience
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    // Enhance docking visual feedback
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    style.TabRounding = 4.0f;
    style.WindowMenuButtonPosition = ImGuiDir_Right;

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

    std::cout << "[OK] ImGui initialized successfully!" << std::endl;
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    // Note: ViewportsEnable is not supported on web/emscripten

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Customize style for better docking experience on web
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    style.TabRounding = 4.0f;
    style.WindowMenuButtonPosition = ImGuiDir_Right;

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

    std::cout << "[OK] ImGui initialized successfully for web (minimal mode)!" << std::endl;
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
    std::cout << "[SHUTDOWN] ImGui shutdown complete" << std::endl;
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

    // Create main dockspace
    setupDockspace();
#endif

    // Show game engine GUI panels (docked around edges, center stays clear for 3D scene)
    if (m_showHierarchy) {
        showHierarchy();
    }

    if (m_showInspector) {
        showInspector();
    }

    if (m_showProject) {
        showProject();
    }

    if (m_showConsole) {
        showConsole();
    }

    if (m_showViewport) {
        showViewportControls();
    }

    if (m_showAssets) {
        showAssets();
    }

    if (m_showSceneViewport) {
        showSceneViewport();
    }

    // Add a floating 3D Scene status overlay - TEMPORARILY DISABLED
    // if (true) { // Always show for debugging
    //     ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    //     ImGui::SetNextWindowSize(ImVec2(300, 120), ImGuiCond_FirstUseEver);
    //     ImGui::Begin("3D Scene Status", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    //     ImGui::Text("🎯 3D Scene Rendering:");
    //     ImGui::Text("✅ Scene initialized with 3 cubes");
    //     ImGui::Text("✅ Shaders loaded successfully");
    //     ImGui::Text("✅ WebGPU renderer active");
    //     ImGui::Text("📍 Look at CENTER of window");
    //     ImGui::Text("   (behind this overlay)");
    //     if (ImGui::Button("Hide This Window")) {
    //         // TODO: Add toggle for this window
    //     }
    //     ImGui::End();
    // }

    // Show debug windows (collapsed by default)
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
    
    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
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
    }

    if (ImGui::CollapsingHeader("System Info")) {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // Show docking status
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Docking: %s", (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) ? "[ON] Enabled" : "[OFF] Disabled");
        ImGui::Text("Multi-Viewport: %s", (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) ? "[ON] Enabled" : "[OFF] Disabled");

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
        ImGui::Text("- Vertex Shaders");
        ImGui::Text("- Fragment Shaders");
        ImGui::Text("- Compute Shaders");
        ImGui::Text("- Render Targets");
        ImGui::Text("- Depth Testing");
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

void ImGuiManager::setupDockspace() {
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Standard docking without passthrough

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("RS Engine DockSpace", &m_dockspaceEnabled, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("RSEngineDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        // First time setup - create default layout
        static bool first_time = true;
        if (first_time) {
            first_time = false;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            
            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

            // Create game engine layout: Hierarchy | 3D Viewport (passthrough) + Inspector | Project/Console
            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);
            
            // Split bottom area for project and console
            auto dock_id_down_left = ImGui::DockBuilderSplitNode(dock_id_down, ImGuiDir_Left, 0.6f, nullptr, &dock_id_down);
            
            // Split left area for hierarchy and assets
            auto dock_id_left_down = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.6f, nullptr, &dock_id_left);

            // Dock main game engine windows with Scene Viewport in center
            ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
            ImGui::DockBuilderDockWindow("Assets", dock_id_left_down);
            ImGui::DockBuilderDockWindow("Scene Viewport", dockspace_id); // Main 3D scene in center
            ImGui::DockBuilderDockWindow("Viewport Controls", dock_id_right);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Project", dock_id_down_left);
            ImGui::DockBuilderDockWindow("Console", dock_id_down);
            
            // Dock debug windows (collapsed by default)
            ImGui::DockBuilderDockWindow("Debug Controls", dock_id_right);
            ImGui::DockBuilderDockWindow("Scene Debugger", dock_id_right);
            ImGui::DockBuilderDockWindow("Performance Metrics", dock_id_down);
            ImGui::DockBuilderDockWindow("WebGPU Info", dock_id_down);
            ImGui::DockBuilderDockWindow("Memory Usage", dock_id_down);
            
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    // Main menu bar
    if (ImGui::BeginMenuBar()) {
        // Add 3D scene status indicator
        ImGui::Text("[ENGINE] RS Engine");
        ImGui::Separator();
        ImGui::Text("3D Scene: [ACTIVE]");
        ImGui::Separator();
        
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Build Settings")) {}
            if (ImGui::MenuItem("Build and Run", "Ctrl+B")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("GameObject")) {
            if (ImGui::MenuItem("Create Empty")) {}
            ImGui::Separator();
            if (ImGui::BeginMenu("3D Object")) {
                if (ImGui::MenuItem("Cube")) {}
                if (ImGui::MenuItem("Sphere")) {}
                if (ImGui::MenuItem("Plane")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Light")) {
                if (ImGui::MenuItem("Directional Light")) {}
                if (ImGui::MenuItem("Point Light")) {}
                if (ImGui::MenuItem("Spot Light")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Window")) {
            ImGui::Text("Main Panels");
            ImGui::Separator();
            ImGui::Checkbox("Scene Viewport", &m_showSceneViewport);
            ImGui::Checkbox("Hierarchy", &m_showHierarchy);
            ImGui::Checkbox("Inspector", &m_showInspector);
            ImGui::Checkbox("Project", &m_showProject);
            ImGui::Checkbox("Console", &m_showConsole);
            ImGui::Checkbox("Viewport Controls", &m_showViewport);
            ImGui::Checkbox("Assets", &m_showAssets);
            ImGui::Separator();
            ImGui::Text("Debug Panels");
            ImGui::Separator();
            ImGui::Checkbox("Debug Controls", &m_showDebugWindow);
            ImGui::Checkbox("Scene Debugger", &m_showSceneDebugger);
            ImGui::Checkbox("Performance Metrics", &m_showPerformanceMetrics);
            ImGui::Checkbox("WebGPU Info", &m_showWebGPUInfo);
            ImGui::Checkbox("Memory Usage", &m_showMemoryUsage);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::Checkbox("Fullscreen Dockspace", &opt_fullscreen);
            ImGui::Checkbox("Window Padding", &opt_padding);
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Layout")) {
                resetDockingLayout();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Engine")) {
            ImGui::Text("RS Engine WebGPU v1.0");
            ImGui::Separator();
            ImGui::Text("Cross-platform rendering engine");
            ImGui::Text("WebGPU + Physics simulation");
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void ImGuiManager::resetDockingLayout() {
    // Force reset the docking layout
    static bool force_reset = true;
    if (force_reset) {
        ImGuiID dockspace_id = ImGui::GetID("RSEngineDockSpace");
        ImGui::DockBuilderRemoveNode(dockspace_id);
        force_reset = false;
        
        std::cout << "[RESET] Docking layout reset - will rebuild on next frame" << std::endl;
    }
}

void ImGuiManager::saveDockingLayout() {
    // TODO: Implement layout saving to file
    // This would typically save the current docking configuration to imgui.ini
    // or a custom configuration file
    std::cout << "[SAVE] Saving docking layout (not implemented yet)" << std::endl;
}

void ImGuiManager::loadDockingLayout() {
    // TODO: Implement layout loading from file
    // This would restore a previously saved docking configuration
    std::cout << "[LOAD] Loading docking layout (not implemented yet)" << std::endl;
}

// Game Engine GUI Panels Implementation
void ImGuiManager::showHierarchy() {
    ImGui::Begin("Hierarchy", &m_showHierarchy);

    ImGui::Text("Scene Hierarchy");
    ImGui::Separator();

    // Scene tree structure
    if (ImGui::TreeNode("Scene")) {
        if (ImGui::TreeNode("Main Camera")) {
            ImGui::Text("Transform");
            ImGui::Text("Camera Component");
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Lighting")) {
            if (ImGui::TreeNode("Directional Light")) {
                ImGui::Text("Transform");
                ImGui::Text("Light Component");
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Geometry")) {
            if (ImGui::TreeNode("Cube 1")) {
                ImGui::Text("Transform");
                ImGui::Text("Mesh Renderer");
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Cube 2")) {
                ImGui::Text("Transform");
                ImGui::Text("Mesh Renderer");
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Cube 3")) {
                ImGui::Text("Transform");
                ImGui::Text("Mesh Renderer");
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }

    ImGui::End();
}

void ImGuiManager::showInspector() {
    ImGui::Begin("Inspector", &m_showInspector);

    ImGui::Text("Object Inspector");
    ImGui::Separator();

    // Object properties (example)
    static char objectName[128] = "Selected Object";
    ImGui::InputText("Name", objectName, sizeof(objectName));
    
    ImGui::Checkbox("Active", &m_showInspector); // Placeholder
    
    ImGui::Separator();
    
    // Transform component
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        static float position[3] = {0.0f, 0.0f, 0.0f};
        static float rotation[3] = {0.0f, 0.0f, 0.0f};
        static float scale[3] = {1.0f, 1.0f, 1.0f};
        
        ImGui::DragFloat3("Position", position, 0.1f);
        ImGui::DragFloat3("Rotation", rotation, 1.0f);
        ImGui::DragFloat3("Scale", scale, 0.1f);
    }
    
    // Mesh Renderer component
    if (ImGui::CollapsingHeader("Mesh Renderer")) {
        static int materialSlot = 0;
        ImGui::Combo("Material", &materialSlot, "Default\0Metal\0Wood\0Glass\0");
        
        static bool castShadows = true;
        static bool receiveShadows = true;
        ImGui::Checkbox("Cast Shadows", &castShadows);
        ImGui::Checkbox("Receive Shadows", &receiveShadows);
    }
    
    // Add Component button
    ImGui::Separator();
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("ComponentMenu");
    }
    
    if (ImGui::BeginPopup("ComponentMenu")) {
        if (ImGui::MenuItem("Rigidbody")) {}
        if (ImGui::MenuItem("Collider")) {}
        if (ImGui::MenuItem("Light")) {}
        if (ImGui::MenuItem("Camera")) {}
        ImGui::EndPopup();
    }

    ImGui::End();
}

void ImGuiManager::showProject() {
    ImGui::Begin("Project", &m_showProject);

    ImGui::Text("Project Browser");
    ImGui::Separator();

    // Project folder structure
    if (ImGui::TreeNode("Assets")) {
        if (ImGui::TreeNode("Meshes")) {
            ImGui::Text("[MESH] cube.obj");
            ImGui::Text("[MESH] sphere.obj");
            ImGui::Text("[MESH] plane.obj");
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Materials")) {
            ImGui::Text("[MAT] DefaultMaterial.mat");
            ImGui::Text("[MAT] MetalMaterial.mat");
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Shaders")) {
            ImGui::Text("[SHDR] vertex_shader.wgsl");
            ImGui::Text("[SHDR] fragment_shader.wgsl");
            ImGui::Text("[SHDR] compute_shader.wgsl");
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Textures")) {
            ImGui::Text("[TEX] diffuse.png");
            ImGui::Text("[TEX] normal.png");
            ImGui::Text("[TEX] roughness.png");
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }

    ImGui::End();
}

void ImGuiManager::showConsole() {
    ImGui::Begin("Console", &m_showConsole);

    ImGui::Text("Console / Log Output");
    ImGui::Separator();

    // Console output area
    static ImGuiTextBuffer console_buffer;
    static bool auto_scroll = true;
    
    if (ImGui::BeginChild("ConsoleOutput", ImVec2(0, -30))) {
        // Sample log messages
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "[INFO] RS Engine WebGPU initialized successfully");
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[SUCCESS] WebGPU device created");
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[SUCCESS] Shaders compiled successfully");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "[WARNING] Using default material");
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "[INFO] Frame rate: %.1f FPS", m_fps);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "[INFO] Frame time: %.3f ms", m_frameTime * 1000.0f);
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::Checkbox("Auto-scroll", &auto_scroll);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        console_buffer.clear();
    }

    ImGui::End();
}

void ImGuiManager::showViewportControls() {
    ImGui::Begin("Viewport Controls", &m_showViewport);

    ImGui::Text("3D Scene Controls");
    ImGui::Separator();

    // Compact viewport controls
    if (ImGui::Button("[FOCUS]")) {}
    ImGui::SameLine();
    if (ImGui::Button("[SHOT]")) {}
    ImGui::SameLine();
    if (ImGui::Button("[RESET]")) {}
    
    ImGui::Separator();
    
    // Camera controls (compact)
    static float fov = 60.0f;
    static float nearPlane = 0.1f;
    static float farPlane = 100.0f;
    
    ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f, "%.0f deg");
    ImGui::DragFloat("Near", &nearPlane, 0.01f, 0.01f, 10.0f, "%.2f");
    ImGui::DragFloat("Far", &farPlane, 1.0f, 1.0f, 1000.0f, "%.0f");
    
    ImGui::Separator();
    
    // Rendering settings (compact)
    static bool wireframe = false;
    static bool showGrid = true;
    static bool showGizmos = true;
    
    ImGui::Checkbox("Wire", &wireframe);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &showGrid);
    ImGui::SameLine();
    ImGui::Checkbox("Gizmo", &showGizmos);
    
    ImGui::Separator();
    ImGui::Text("Center: 3D Scene");
    ImGui::Text("FPS: %.1f", m_fps);

    ImGui::End();
}

void ImGuiManager::showAssets() {
    ImGui::Begin("Assets", &m_showAssets);

    ImGui::Text("Asset Browser");
    ImGui::Separator();

    // Filter controls
    static char filter[128] = "";
    ImGui::InputText("Search", filter, sizeof(filter));
    
    static int assetTypeFilter = 0;
    ImGui::Combo("Type", &assetTypeFilter, "All\0Meshes\0Materials\0Shaders\0Textures\0");
    
    ImGui::Separator();

    // Asset thumbnails/list
    ImGui::Columns(3, "AssetColumns");
    
    // Sample assets
    ImGui::Text("[MESH]");
    ImGui::Text("cube.obj");
    ImGui::NextColumn();
    
    ImGui::Text("[MAT]");
    ImGui::Text("default.mat");
    ImGui::NextColumn();
    
    ImGui::Text("[SHDR]");
    ImGui::Text("vertex.wgsl");
    ImGui::NextColumn();
    
    ImGui::Text("[MESH]");
    ImGui::Text("sphere.obj");
    ImGui::NextColumn();
    
    ImGui::Text("[TEX]");
    ImGui::Text("texture.png");
    ImGui::NextColumn();
    
    ImGui::Text("[SHDR]");
    ImGui::Text("fragment.wgsl");
    ImGui::NextColumn();
    
    ImGui::Columns(1);

    ImGui::End();
}

void ImGuiManager::showSceneViewport() {
    ImGui::Begin("Scene Viewport", &m_showSceneViewport);

    // Get available region for rendering
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    if (m_application) {
        // Get the 3D scene texture from the application
        wgpu::TextureView sceneTextureView = m_application->getSceneTextureView();

        if (sceneTextureView) {
            // Create or update texture ID for ImGui
            if (!m_sceneTextureID) {
                // Create texture binding for ImGui WebGPU backend
                m_sceneTextureID = (void*)sceneTextureView.Get();
            }

            // Use the entire available space for the 3D scene
            ImVec2 displaySize = viewportPanelSize;

            // Display the actual 3D scene texture using ImGui::Image
            if (m_sceneTextureID) {
                // Use ImGui::Image to display the WebGPU texture (fills entire window)
                ImGui::Image(m_sceneTextureID, displaySize);
            } else {
                // Fallback if texture ID creation failed
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos();

                draw_list->AddRectFilled(canvas_pos,
                                        ImVec2(canvas_pos.x + displaySize.x, canvas_pos.y + displaySize.y),
                                        IM_COL32(120, 50, 50, 255)); // Reddish to indicate problem

                draw_list->AddRect(canvas_pos,
                                  ImVec2(canvas_pos.x + displaySize.x, canvas_pos.y + displaySize.y),
                                  IM_COL32(150, 150, 150, 255), 0.0f, 0, 2.0f);

                ImVec2 text_pos = ImVec2(canvas_pos.x + displaySize.x * 0.5f - 80,
                                        canvas_pos.y + displaySize.y * 0.5f);
                draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), "Texture Binding Failed");

                ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x, canvas_pos.y + displaySize.y + 8));
                ImGui::Dummy(ImVec2(displaySize.x, 1.0f));
            }

        } else {
            ImGui::Text("[ERROR] No scene texture available");
            ImGui::Text("Waiting for render target initialization...");
        }
    } else {
        ImGui::Text("[ERROR] No application reference");
        ImGui::Text("ImGui manager not properly connected to application");
    }

    ImGui::End();
}

void ImGuiManager::onWindowResize(int width, int height) {
    if (!m_initialized) {
        return;
    }

#ifdef __EMSCRIPTEN__
    // For web builds, update display size manually
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
#else
    // For native builds, ImGui GLFW backend handles this automatically
    // through ImGui_ImplGlfw_NewFrame(), but we can force an update if needed
    if (m_window) {
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)display_w, (float)display_h);

        if (display_w > 0 && display_h > 0) {
            io.DisplayFramebufferScale = ImVec2((float)display_w / width, (float)display_h / height);
        }
    }
#endif
}

} // namespace gui
} // namespace rs_engine