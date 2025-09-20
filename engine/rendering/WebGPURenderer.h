#pragma once

#include "../core/Config.h"

#ifdef __EMSCRIPTEN__
    #include <webgpu/webgpu.h>
    #include <webgpu/webgpu_cpp.h>
#else
    #include <dawn/webgpu_cpp.h>
#endif

namespace rs_engine {

class WebGPURenderer {
private:
    wgpu::Device* device;
    uint32_t maxBufferSize;
    uint32_t workgroupSize;

public:
    WebGPURenderer(wgpu::Device* dev) : device(dev) {
        auto limits = EngineConfig::getLimits();
        maxBufferSize = limits.maxBufferSize;
        workgroupSize = limits.workgroupSize;
    }

    wgpu::Buffer createBuffer(size_t size, wgpu::BufferUsage usage) {
        if (size > maxBufferSize) {
            size = maxBufferSize;
        }

        wgpu::BufferDescriptor desc{};
        desc.size = (size + 15) & ~15; // 16-byte alignment
        desc.usage = usage;
        return device->CreateBuffer(&desc);
    }

    wgpu::ComputePipeline createComputePipeline(const std::string& shaderCode) {
        std::string defines = generatePlatformDefines();
        std::string finalCode = defines + "\n" + shaderCode;

        wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
        wgslDesc.code = finalCode.c_str();

        wgpu::ShaderModuleDescriptor shaderDesc{};
        shaderDesc.nextInChain = &wgslDesc;

        auto shaderModule = device->CreateShaderModule(&shaderDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc{};
        pipelineDesc.compute.module = shaderModule;
        pipelineDesc.compute.entryPoint = "main";

        return device->CreateComputePipeline(&pipelineDesc);
    }

private:
    std::string generatePlatformDefines() {
        auto limits = EngineConfig::getLimits();
        return
            "#define MAX_PARTICLES " + std::to_string(limits.maxParticles) + "\n" +
            "#define WORKGROUP_SIZE " + std::to_string(limits.workgroupSize) + "\n" +
            "#define ENABLE_ADVANCED_FEATURES " + std::to_string(limits.enableAdvancedFeatures ? 1 : 0) + "\n";
    }
};

} // namespace rs_engine