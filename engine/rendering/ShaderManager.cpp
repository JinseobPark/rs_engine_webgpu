#include "ShaderManager.h"
#include <iostream>
#include <filesystem>

#ifdef __EMSCRIPTEN__
#include "rendering/EmbeddedShaders.h"
#endif

namespace rs_engine {

std::string ShaderManager::loadShaderFile(const std::string& filePath) {
#ifdef __EMSCRIPTEN__
    // For web builds, use generated embedded shaders
    const std::string& shaderCode = EmbeddedShaders::getShader(filePath);
    if (shaderCode.empty()) {
        std::cerr << "Embedded shader not found: " << filePath << std::endl;
        return "";
    }
    std::cout << "Loaded embedded shader: " << filePath << " (" << shaderCode.length() << " chars)" << std::endl;
    return shaderCode;
#else
    // For native builds, load from file system
    std::string fullPath = shaderBasePath + filePath;

    std::cout << "Attempting to load shader: " << fullPath << std::endl;

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << fullPath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();
    std::cout << "Successfully loaded shader: " << fullPath << " (" << content.length() << " chars)" << std::endl;

    return content;
#endif
}

std::string ShaderManager::preprocessShader(const std::string& shaderCode, const std::string& filePath) {
    std::string defines;
    
    // Only add compute-specific defines for compute shaders
    if (filePath.find("compute/") != std::string::npos) {
        auto limits = EngineConfig::getLimits();
        defines =
            "#define MAX_PARTICLES " + std::to_string(limits.maxParticles) + "\n" +
            "#define WORKGROUP_SIZE " + std::to_string(limits.workgroupSize) + "\n" +
            "#define ENABLE_ADVANCED_FEATURES " + std::to_string(limits.enableAdvancedFeatures ? 1 : 0) + "\n";
    }
    
    // For render shaders, we might add different defines in the future if needed
    // Currently, render shaders don't need platform-specific defines

    return defines.empty() ? shaderCode : defines + "\n" + shaderCode;
}

wgpu::ShaderModule ShaderManager::loadShader(const std::string& filePath) {
    auto it = shaderCache.find(filePath);
    if (it != shaderCache.end()) {
        return it->second;
    }

    std::string shaderCode = loadShaderFile(filePath);
    if (shaderCode.empty()) {
        return nullptr;
    }

    wgpu::ShaderModule module = createShaderFromCode(shaderCode, filePath);
    if (module) {
        shaderCache[filePath] = module;
    }

    return module;
}

wgpu::ShaderModule ShaderManager::createShaderFromCode(const std::string& shaderCode, const std::string& name) {
    std::string processedCode = preprocessShader(shaderCode, name);

    wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
    wgslDesc.code = processedCode.c_str();

    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &wgslDesc;
    if (!name.empty()) {
        shaderDesc.label = name.c_str();
    }

    wgpu::ShaderModule module = device->CreateShaderModule(&shaderDesc);
    if (!module) {
        std::cerr << "Failed to create shader module" << (name.empty() ? "" : " for " + name) << std::endl;
        std::cerr << "Processed shader code:" << std::endl;
        std::cerr << processedCode << std::endl;
    }

    return module;
}

wgpu::RenderPipeline ShaderManager::createRenderPipeline(
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath,
    const wgpu::TextureFormat& colorFormat,
    bool hasVertexBuffer
) {
    wgpu::ShaderModule vertexShader = loadShader(vertexShaderPath);
    wgpu::ShaderModule fragmentShader = loadShader(fragmentShaderPath);

    if (!vertexShader || !fragmentShader) {
        std::cerr << "Failed to load shaders for pipeline" << std::endl;
        return nullptr;
    }

    wgpu::RenderPipelineDescriptor pipelineDesc{};

    // Vertex stage
    pipelineDesc.vertex.module = vertexShader;
    pipelineDesc.vertex.entryPoint = "vs_main";

    // Fragment stage
    wgpu::FragmentState fragmentState{};
    fragmentState.module = fragmentShader;
    fragmentState.entryPoint = "fs_main";

    // Color target
    wgpu::ColorTargetState colorTarget{};
    colorTarget.format = colorFormat;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;

    // Vertex buffer layout (if needed)
    wgpu::VertexBufferLayout vertexBufferLayout{};
    std::vector<wgpu::VertexAttribute> attributes;

    if (hasVertexBuffer) {
        // Position attribute (3 floats)
        wgpu::VertexAttribute positionAttr{};
        positionAttr.format = wgpu::VertexFormat::Float32x3;
        positionAttr.offset = 0;
        positionAttr.shaderLocation = 0;
        attributes.push_back(positionAttr);

        vertexBufferLayout.arrayStride = 3 * sizeof(float);
        vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
        vertexBufferLayout.attributeCount = attributes.size();
        vertexBufferLayout.attributes = attributes.data();

        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertexBufferLayout;
    }

    // Primitive state
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
    pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

    // Multisample state
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    wgpu::RenderPipeline pipeline = device->CreateRenderPipeline(&pipelineDesc);
    if (!pipeline) {
        std::cerr << "Failed to create render pipeline" << std::endl;
    }

    return pipeline;
}

void ShaderManager::clearCache() {
    shaderCache.clear();
}

} // namespace rs_engine