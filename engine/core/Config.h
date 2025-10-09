#pragma once

#include <cstdint>

namespace rs_engine {

struct PlatformLimits {
    uint32_t maxParticles;
    uint32_t maxBufferSize;
    uint32_t workgroupSize;
    bool enableMultithreading;
    bool enableAdvancedFeatures;
};

/**
 * @brief Object picking configuration
 */
struct PickingConfig {
    // Always use AABB filtering then precise triangle intersection
    uint32_t maxCandidates = 5;  // Test top N AABB candidates
    
    PickingConfig() = default;
};

class EngineConfig {
private:
    static constexpr PlatformLimits getPlatformLimits() {
        #ifdef __EMSCRIPTEN__
            return {
                .maxParticles = 32768,
                .maxBufferSize = 64 * 1024 * 1024,
                .workgroupSize = 64,
                .enableMultithreading = false,
                .enableAdvancedFeatures = false
            };
        #else
            return {
                .maxParticles = 262144,
                .maxBufferSize = 512 * 1024 * 1024,
                .workgroupSize = 128,
                .enableMultithreading = true,
                .enableAdvancedFeatures = true
            };
        #endif
    }

public:
    static const PlatformLimits& getLimits() {
        static const PlatformLimits limits = getPlatformLimits();
        return limits;
    }

    static uint32_t getOptimalParticleCount(float qualityLevel = 1.0f) {
        return static_cast<uint32_t>(getLimits().maxParticles * qualityLevel);
    }
    
    static const PickingConfig& getPickingConfig() {
        static PickingConfig config;
        return config;
    }
};

} // namespace rs_engine