#include <GLFW/glfw3.h>
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <dawn/webgpu_cpp.h>
#include <webgpu/webgpu_glfw.h>
#include <iostream>
#include <vector>
#include <cassert>

struct TriangleApp {
    GLFWwindow* window = nullptr;
    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Surface surface = nullptr;
    wgpu::RenderPipeline pipeline = nullptr;
    wgpu::Buffer vertexBuffer = nullptr;

    static void errorCallback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    bool initWebGPU() {
        // Initialize Dawn procedures
        DawnProcTable procs = dawn::native::GetProcs();
        dawnProcSetProcs(&procs);

        // Create WebGPU instance
        wgpu::InstanceDescriptor instanceDesc = {};
        instance = wgpu::CreateInstance(&instanceDesc);
        if (!instance) {
            std::cerr << "Failed to create WebGPU instance" << std::endl;
            return false;
        }

        // Create surface for the window
        surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
        if (!surface) {
            std::cerr << "Failed to create surface" << std::endl;
            return false;
        }

        // Get adapter
        dawn::native::Instance* nativeInstance = reinterpret_cast<dawn::native::Instance*>(instance.Get());
        std::vector<dawn::native::Adapter> adapters = nativeInstance->EnumerateAdapters();
        
        if (adapters.empty()) {
            std::cerr << "No adapters found" << std::endl;
            return false;
        }

        adapter = wgpu::Adapter(adapters[0].Get());

        // Create device (no error callback in modern API - errors are handled differently)
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.label = "Main Device";
        device = adapter.CreateDevice(&deviceDesc);
        if (!device) {
            std::cerr << "Failed to create device" << std::endl;
            return false;
        }

        return true;
    }

    bool createRenderPipeline() {
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

    void configureSurface() {
        wgpu::SurfaceConfiguration surfaceConfig = {};
        surfaceConfig.device = device;
        surfaceConfig.format = wgpu::TextureFormat::BGRA8Unorm;
        surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
        surfaceConfig.width = 800;
        surfaceConfig.height = 600;
        surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
        
        surface.Configure(&surfaceConfig);
    }

    void render() {
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

        surface.Present();
    }

    bool init() {
        // Initialize GLFW
        glfwSetErrorCallback(errorCallback);
        
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        window = glfwCreateWindow(800, 600, "WebGPU Triangle - Native", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwSetKeyCallback(window, keyCallback);

        if (!initWebGPU()) {
            return false;
        }

        if (!createRenderPipeline()) {
            return false;
        }

        configureSurface();

        std::cout << "WebGPU Triangle App (Native) initialized successfully!" << std::endl;
        std::cout << "Press ESC to close the window." << std::endl;

        return true;
    }

    void run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            render();
        }
    }

    void cleanup() {
        if (window) {
            glfwDestroyWindow(window);
            glfwTerminate();
        }
        std::cout << "Application terminated successfully." << std::endl;
    }
};

int main() {
    TriangleApp app;
    
    if (!app.init()) {
        app.cleanup();
        return -1;
    }

    app.run();
    app.cleanup();
    
    return 0;
}
