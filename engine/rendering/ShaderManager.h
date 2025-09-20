#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include "../core/Config.h"

#ifdef __EMSCRIPTEN__
    #include <webgpu/webgpu.h>
    #include <webgpu/webgpu_cpp.h>
#else
    #include <dawn/webgpu_cpp.h>
#endif

namespace rs_engine {

class ShaderManager {
private:
    wgpu::Device* device;
    std::unordered_map<std::string, wgpu::ShaderModule> shaderCache;
    std::string shaderBasePath;

public:
    ShaderManager(wgpu::Device* dev, const std::string& basePath = "shaders/")
        : device(dev), shaderBasePath(basePath) {}

    wgpu::ShaderModule loadShader(const std::string& filePath);
    wgpu::ShaderModule createShaderFromCode(const std::string& shaderCode, const std::string& name = "");
    wgpu::RenderPipeline createRenderPipeline(
        const std::string& vertexShaderPath,
        const std::string& fragmentShaderPath,
        const wgpu::TextureFormat& colorFormat = wgpu::TextureFormat::BGRA8Unorm,
        bool hasVertexBuffer = false
    );

    void clearCache();

private:
    std::string loadShaderFile(const std::string& filePath);
    std::string preprocessShader(const std::string& shaderCode, const std::string& filePath = "");
};

} // namespace rs_engine