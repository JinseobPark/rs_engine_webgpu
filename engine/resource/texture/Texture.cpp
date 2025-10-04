#include "Texture.h"
#include <iostream>
#include <cstring>

// TODO: Add stb_image for actual image loading
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

namespace rs_engine {
namespace resource {

Texture::Texture() {
    metadata.type = ResourceType::Texture;
    metadata.state = ResourceState::Unloaded;
}

Texture::Texture(const std::string& name) : Texture() {
    metadata.name = name;
}

Texture::~Texture() {
    unload();
}

bool Texture::load() {
    if (metadata.state == ResourceState::Loaded) {
        return true;
    }
    
    // For procedural textures, data is already in memory
    if (!pixelData.empty() && width > 0 && height > 0) {
        metadata.state = ResourceState::Loaded;
        metadata.memorySize = pixelData.size();
        return true;
    }
    
    // For file-based textures
    if (!metadata.filepath.empty()) {
        return loadFromFile(metadata.filepath);
    }
    
    metadata.state = ResourceState::Failed;
    return false;
}

void Texture::unload() {
    releaseGPUResources();
    pixelData.clear();
    width = height = channels = 0;
    format = TextureFormat::Unknown;
    metadata.state = ResourceState::Unloaded;
    metadata.memorySize = 0;
}

void Texture::setData(const uint8_t* data, uint32_t w, uint32_t h, uint32_t ch) {
    if (!data || w == 0 || h == 0 || ch == 0) {
        return;
    }
    
    width = w;
    height = h;
    channels = ch;
    
    // Determine format
    switch (channels) {
        case 1: format = TextureFormat::R8; break;
        case 2: format = TextureFormat::RG8; break;
        case 3: format = TextureFormat::RGB8; break;
        case 4: format = TextureFormat::RGBA8; break;
        default: format = TextureFormat::Unknown; break;
    }
    
    size_t dataSize = w * h * ch;
    pixelData.resize(dataSize);
    std::memcpy(pixelData.data(), data, dataSize);
    
    gpuDataCreated = false; // Need to recreate GPU resources
}

bool Texture::loadFromFile(const std::string& filepath) {
    // TODO: Implement with stb_image
    // For now, return placeholder implementation
    
    std::cerr << "Texture::loadFromFile() not yet implemented: " << filepath << std::endl;
    
    /*
    int w, h, ch;
    unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &ch, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        metadata.state = ResourceState::Failed;
        return false;
    }
    
    setData(data, w, h, ch);
    stbi_image_free(data);
    
    metadata.filepath = filepath;
    metadata.state = ResourceState::Loaded;
    return true;
    */
    
    return false;
}

bool Texture::createGPUResources(wgpu::Device device) {
    if (!device || pixelData.empty() || width == 0 || height == 0) {
        return false;
    }
    
    // Release old resources
    releaseGPUResources();
    
    // Create texture
    wgpu::TextureDescriptor textureDesc;
    textureDesc.size = {width, height, 1};
    textureDesc.format = getWebGPUFormat();
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | 
                       wgpu::TextureUsage::CopyDst |
                       wgpu::TextureUsage::RenderAttachment;
    textureDesc.mipLevelCount = 1; // TODO: Generate mipmaps if requested
    textureDesc.sampleCount = 1;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    
    gpuTexture = device.CreateTexture(&textureDesc);
    if (!gpuTexture) {
        std::cerr << "Failed to create GPU texture: " << metadata.name << std::endl;
        return false;
    }
    
    // Upload pixel data via buffer (workaround for API compatibility)
    // TODO: Use WriteTexture when API stabilizes
    // For now, we'll mark as created but data upload will be handled separately
    std::cout << "⚠️  Texture GPU upload needs WriteTexture API implementation" << std::endl;
    
    // Create texture view
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = textureDesc.format;
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = wgpu::TextureAspect::All;
    
    textureView = gpuTexture.CreateView(&viewDesc);
    if (!textureView) {
        std::cerr << "Failed to create texture view: " << metadata.name << std::endl;
        gpuTexture = nullptr;
        return false;
    }
    
    // Create sampler
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = getWebGPUWrapMode();
    samplerDesc.addressModeV = getWebGPUWrapMode();
    samplerDesc.addressModeW = getWebGPUWrapMode();
    samplerDesc.magFilter = getWebGPUFilterMode();
    samplerDesc.minFilter = getWebGPUFilterMode();
    samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 1.0f;
    samplerDesc.compare = wgpu::CompareFunction::Undefined;
    samplerDesc.maxAnisotropy = 1;
    
    sampler = device.CreateSampler(&samplerDesc);
    if (!sampler) {
        std::cerr << "Failed to create sampler: " << metadata.name << std::endl;
        textureView = nullptr;
        gpuTexture = nullptr;
        return false;
    }
    
    gpuDataCreated = true;
    return true;
}

void Texture::releaseGPUResources() {
    if (sampler) {
        sampler = nullptr;
    }
    if (textureView) {
        textureView = nullptr;
    }
    if (gpuTexture) {
        gpuTexture.Destroy();
        gpuTexture = nullptr;
    }
    gpuDataCreated = false;
}

wgpu::TextureFormat Texture::getWebGPUFormat() const {
    switch (format) {
        case TextureFormat::R8: return wgpu::TextureFormat::R8Unorm;
        case TextureFormat::RG8: return wgpu::TextureFormat::RG8Unorm;
        case TextureFormat::RGB8: return wgpu::TextureFormat::RGBA8Unorm; // RGB not supported, use RGBA
        case TextureFormat::RGBA8: return wgpu::TextureFormat::RGBA8Unorm;
        case TextureFormat::R16F: return wgpu::TextureFormat::R16Float;
        case TextureFormat::RGBA16F: return wgpu::TextureFormat::RGBA16Float;
        case TextureFormat::R32F: return wgpu::TextureFormat::R32Float;
        case TextureFormat::RGBA32F: return wgpu::TextureFormat::RGBA32Float;
        default: return wgpu::TextureFormat::RGBA8Unorm;
    }
}

wgpu::FilterMode Texture::getWebGPUFilterMode() const {
    switch (filterMode) {
        case TextureFilter::Nearest: return wgpu::FilterMode::Nearest;
        case TextureFilter::Linear:
        case TextureFilter::Bilinear:
        case TextureFilter::Trilinear:
            return wgpu::FilterMode::Linear;
        default: return wgpu::FilterMode::Linear;
    }
}

wgpu::AddressMode Texture::getWebGPUWrapMode() const {
    switch (wrapMode) {
        case TextureWrap::Repeat: return wgpu::AddressMode::Repeat;
        case TextureWrap::Clamp: return wgpu::AddressMode::ClampToEdge;
        case TextureWrap::Mirror: return wgpu::AddressMode::MirrorRepeat;
        default: return wgpu::AddressMode::Repeat;
    }
}

// ========== Static Factory Methods ==========

Texture* Texture::createSolidColor(const std::string& name,
                                   uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                   uint32_t width, uint32_t height) {
    Texture* texture = new Texture(name);
    
    std::vector<uint8_t> data(width * height * 4);
    for (uint32_t i = 0; i < width * height; ++i) {
        data[i * 4 + 0] = r;
        data[i * 4 + 1] = g;
        data[i * 4 + 2] = b;
        data[i * 4 + 3] = a;
    }
    
    texture->setData(data.data(), width, height, 4);
    texture->load();
    
    return texture;
}

Texture* Texture::createCheckerboard(const std::string& name,
                                     uint32_t size, uint32_t checkSize) {
    Texture* texture = new Texture(name);
    
    std::vector<uint8_t> data(size * size * 4);
    
    for (uint32_t y = 0; y < size; ++y) {
        for (uint32_t x = 0; x < size; ++x) {
            bool isWhite = ((x / checkSize) + (y / checkSize)) % 2 == 0;
            uint8_t color = isWhite ? 255 : 0;
            
            uint32_t index = (y * size + x) * 4;
            data[index + 0] = color;
            data[index + 1] = color;
            data[index + 2] = color;
            data[index + 3] = 255;
        }
    }
    
    texture->setData(data.data(), size, size, 4);
    texture->load();
    
    return texture;
}

} // namespace resource
} // namespace rs_engine
