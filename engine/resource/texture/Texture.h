#pragma once

#include "../ResourceTypes.h"
#include <webgpu/webgpu_cpp.h>
#include <vector>
#include <cstdint>

namespace rs_engine {
namespace resource {

/**
 * @brief Texture format types
 */
enum class TextureFormat {
    Unknown,
    R8,           // 8-bit grayscale
    RG8,          // 8-bit 2-channel
    RGB8,         // 8-bit RGB
    RGBA8,        // 8-bit RGBA
    R16F,         // 16-bit float grayscale
    RGBA16F,      // 16-bit float RGBA
    R32F,         // 32-bit float grayscale
    RGBA32F       // 32-bit float RGBA
};

/**
 * @brief Texture filtering options
 */
enum class TextureFilter {
    Nearest,      // Point sampling
    Linear,       // Linear interpolation
    Bilinear,     // Linear + mipmap linear
    Trilinear     // Linear + mipmap linear
};

/**
 * @brief Texture wrap modes
 */
enum class TextureWrap {
    Repeat,       // Repeat texture
    Clamp,        // Clamp to edge
    Mirror        // Mirror repeat
};

/**
 * @brief Texture resource - 2D image data
 * 
 * Platform Support: 100% shared
 * - Image loading: stb_image (cross-platform)
 * - GPU texture: WebGPU (both Web and Native)
 */
class Texture : public IResource {
private:
    // CPU-side data
    std::vector<uint8_t> pixelData;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 0;
    TextureFormat format = TextureFormat::Unknown;
    
    // GPU-side data
    wgpu::Texture gpuTexture;
    wgpu::TextureView textureView;
    wgpu::Sampler sampler;
    bool gpuDataCreated = false;
    
    // Texture settings
    TextureFilter filterMode = TextureFilter::Linear;
    TextureWrap wrapMode = TextureWrap::Repeat;
    bool generateMipmaps = false;

public:
    Texture();
    Texture(const std::string& name);
    virtual ~Texture();
    
    // IResource interface
    bool load() override;
    void unload() override;
    
    // ========== Data Access ==========
    
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    uint32_t getChannels() const { return channels; }
    TextureFormat getFormat() const { return format; }
    
    const std::vector<uint8_t>& getPixelData() const { return pixelData; }
    
    // ========== Data Modification ==========
    
    /**
     * @brief Set texture data from raw pixel array
     * @param data Pixel data
     * @param w Width in pixels
     * @param h Height in pixels
     * @param ch Number of channels (1, 2, 3, or 4)
     */
    void setData(const uint8_t* data, uint32_t w, uint32_t h, uint32_t ch);
    
    /**
     * @brief Load texture from file
     * @param filepath Path to image file (PNG, JPG, etc.)
     * @return true if successful
     */
    bool loadFromFile(const std::string& filepath);
    
    // ========== Texture Settings ==========
    
    void setFilterMode(TextureFilter filter) { filterMode = filter; }
    void setWrapMode(TextureWrap wrap) { wrapMode = wrap; }
    void setGenerateMipmaps(bool generate) { generateMipmaps = generate; }
    
    TextureFilter getFilterMode() const { return filterMode; }
    TextureWrap getWrapMode() const { return wrapMode; }
    bool shouldGenerateMipmaps() const { return generateMipmaps; }
    
    // ========== GPU Resources ==========
    
    /**
     * @brief Create GPU texture from CPU data
     * @param device WebGPU device
     * @return true if successful
     */
    bool createGPUResources(wgpu::Device device);
    
    /**
     * @brief Release GPU texture
     */
    void releaseGPUResources();
    
    wgpu::Texture getGPUTexture() const { return gpuTexture; }
    wgpu::TextureView getTextureView() const { return textureView; }
    wgpu::Sampler getSampler() const { return sampler; }
    bool hasGPUResources() const { return gpuDataCreated; }
    
    // ========== Utility ==========
    
    /**
     * @brief Create a solid color texture
     * @param name Texture name
     * @param color RGBA color (0-255)
     * @param width Texture width
     * @param height Texture height
     */
    static Texture* createSolidColor(const std::string& name,
                                     uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255,
                                     uint32_t width = 1, uint32_t height = 1);
    
    /**
     * @brief Create a checkerboard pattern
     * @param name Texture name
     * @param size Texture size (square)
     * @param checkSize Size of each check in pixels
     */
    static Texture* createCheckerboard(const std::string& name,
                                       uint32_t size = 256,
                                       uint32_t checkSize = 32);

private:
    wgpu::TextureFormat getWebGPUFormat() const;
    wgpu::FilterMode getWebGPUFilterMode() const;
    wgpu::AddressMode getWebGPUWrapMode() const;
};

} // namespace resource
} // namespace rs_engine
