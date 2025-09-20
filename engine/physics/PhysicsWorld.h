#pragma once

#include "../core/Config.h"
#include "../rendering/WebGPURenderer.h"
#include <memory>

namespace rs_engine {

class PhysicsWorld {
private:
    std::unique_ptr<WebGPURenderer> renderer;
    float currentQuality = 1.0f;
    uint32_t activeParticleCount;

public:
    PhysicsWorld(wgpu::Device* device) {
        renderer = std::make_unique<WebGPURenderer>(device);
        activeParticleCount = EngineConfig::getOptimalParticleCount(currentQuality);
    }

    void setQuality(float quality) {
        currentQuality = std::clamp(quality, 0.1f, 1.0f);
        activeParticleCount = EngineConfig::getOptimalParticleCount(currentQuality);
    }

    void update(float deltaTime) {
        // Physics simulation update
        // Implementation will be added later
    }

    uint32_t getActiveParticleCount() const {
        return activeParticleCount;
    }

    float getCurrentQuality() const {
        return currentQuality;
    }

    void adjustQualityForPerformance(float frameTime) {
        const float targetFrameTime = 0.016f; // 60fps

        if (frameTime > targetFrameTime * 1.5f) {
            setQuality(currentQuality * 0.9f);
        } else if (frameTime < targetFrameTime * 0.8f) {
            setQuality(currentQuality * 1.1f);
        }
    }
};

} // namespace rs_engine