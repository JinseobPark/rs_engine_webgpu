#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <iostream>
#include <cassert>

struct TriangleApp {
    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Surface surface = nullptr;
    wgpu::RenderPipeline pipeline = nullptr;
    const char* canvasSelector = "#canvas";

    static void onDeviceError(WGPUErrorType type, char const* message, void* userdata) {
        std::cerr << "Device error: " << message << std::endl;
    }

    static void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata) {
        if (status == WGPURequestAdapterStatus_Success) {
            TriangleApp* app = static_cast<TriangleApp*>(userdata);
            app->adapter = wgpu::Adapter::Acquire(adapter);
            app->onAdapterReceived();
        } else {
            std::cerr << "Could not get WebGPU adapter: " << message << std::endl;
        }
    }

    static void onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
        if (status == WGPURequestDeviceStatus_Success) {
            TriangleApp* app = static_cast<TriangleApp*>(userdata);
            app->device = wgpu::Device::Acquire(device);
            app->onDeviceReceived();
        } else {
            std::cerr << "Could not get WebGPU device: " << message << std::endl;
        }
    }

    void onAdapterReceived() {
        std::cout << "Adapter received, requesting device..." << std::endl;
        
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.label = "Web Device";
        
        adapter.RequestDevice(&deviceDesc, onDeviceRequestEnded, this);
    }

    void onDeviceReceived() {
        std::cout << "Device received, setting up rendering..." << std::endl;
        
        // Set error callback
        device.SetUncapturedErrorCallback(onDeviceError, this);
        
        if (!createSurface()) {
            std::cerr << "Failed to create surface" << std::endl;
            return;
        }
        
        if (!createRenderPipeline()) {
            std::cerr << "Failed to create render pipeline" << std::endl;
            return;
        }
        
        configureSurface();
        startRenderLoop();
        
        std::cout << "WebGPU Triangle App (Web) initialized successfully!" << std::endl;
    }

    bool createSurface() {
        // For Emscripten/Web, create surface using canvas HTML selector
        wgpu::SurfaceDescriptorFromCanvasHTMLSelector htmlSurfaceDesc = {};
        htmlSurfaceDesc.selector = canvasSelector;
        
        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = reinterpret_cast<wgpu::ChainedStruct*>(&htmlSurfaceDesc);
        
        surface = instance.CreateSurface(&surfaceDesc);
        return surface != nullptr;
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
    return vec4f(0.2, 0.8, 0.2, 1.0);
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
        return pipeline != nullptr;
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
        colorAttachment.clearValue = {0.1, 0.2, 0.3, 1.0};

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
    }

    static void renderLoop(void* userData) {
        TriangleApp* app = static_cast<TriangleApp*>(userData);
        app->render();
    }

    void startRenderLoop() {
        emscripten_set_main_loop_arg(renderLoop, this, 60, 1);
    }

    bool init() {
        // Create WebGPU instance (emscripten requires nullptr)
        instance = wgpu::CreateInstance(nullptr);
        if (!instance) {
            std::cerr << "Failed to create WebGPU instance" << std::endl;
            return false;
        }

        // Request adapter
        wgpu::RequestAdapterOptions adapterOpts = {};
        adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;
        
        instance.RequestAdapter(&adapterOpts, onAdapterRequestEnded, this);

        return true;
    }
};

// Global app instance
TriangleApp app;

int main() {
    if (!app.init()) {
        std::cerr << "Failed to initialize WebGPU Triangle App (Web)" << std::endl;
        return -1;
    }

    return 0;
}
