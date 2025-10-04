#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>

namespace rs_engine {
namespace resource {

/**
 * @brief Resource handle type for type-safe resource references
 */
using ResourceHandle = uint64_t;
constexpr ResourceHandle INVALID_RESOURCE_HANDLE = 0;

/**
 * @brief Resource types supported by the engine
 */
enum class ResourceType {
    Unknown,
    Model,
    Mesh,
    Texture,
    Shader,
    Material
};

/**
 * @brief Resource loading state
 */
enum class ResourceState {
    Unloaded,       // Not loaded yet
    Loading,        // Currently being loaded
    Loaded,         // Successfully loaded
    Failed          // Failed to load
};

/**
 * @brief Base resource metadata
 */
struct ResourceMetadata {
    ResourceHandle handle = INVALID_RESOURCE_HANDLE;
    std::string name;
    std::string filepath;
    ResourceType type = ResourceType::Unknown;
    ResourceState state = ResourceState::Unloaded;
    size_t memorySize = 0;  // Memory usage in bytes
    
    ResourceMetadata() = default;
    ResourceMetadata(const std::string& name, const std::string& path, ResourceType type)
        : name(name), filepath(path), type(type) {}
    
    bool isValid() const { return handle != INVALID_RESOURCE_HANDLE; }
    bool isLoaded() const { return state == ResourceState::Loaded; }
};

/**
 * @brief Base class for all resources
 */
class IResource {
public:
    ResourceMetadata metadata;  // Public for easy access by ResourceManager
    
    virtual ~IResource() = default;
    
    const ResourceMetadata& getMetadata() const { return metadata; }
    ResourceHandle getHandle() const { return metadata.handle; }
    const std::string& getName() const { return metadata.name; }
    const std::string& getFilePath() const { return metadata.filepath; }
    ResourceType getType() const { return metadata.type; }
    ResourceState getState() const { return metadata.state; }
    size_t getMemorySize() const { return metadata.memorySize; }
    
    virtual bool load() = 0;
    virtual void unload() = 0;
};

} // namespace resource
} // namespace rs_engine
