#pragma once

#include "../ResourceTypes.h"
#include "../../core/math/Vec3.h"
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace rs_engine {
namespace resource {

/**
 * @brief Vertex data structure for mesh rendering
 */
struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec3 texCoord;  // Using Vec3 for compatibility, only x,y used
    Vec3 color;     // Vertex color (optional)
    
    Vertex() = default;
    Vertex(const Vec3& pos, const Vec3& norm = Vec3(0, 1, 0), 
           const Vec3& uv = Vec3(0, 0, 0), const Vec3& col = Vec3(1, 1, 1))
        : position(pos), normal(norm), texCoord(uv), color(col) {}
};

/**
 * @brief Mesh resource - contains vertex and index data
 * 
 * Platform Support: 100% shared
 * - Vertex data: identical on all platforms
 * - GPU buffers: created via WebGPU (both Web and Native)
 */
class Mesh : public IResource {
private:
    // CPU-side data
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // GPU-side data
    wgpu::Buffer vertexBuffer;
    wgpu::Buffer indexBuffer;
    bool gpuDataCreated = false;

public:
    Mesh();
    Mesh(const std::string& name);
    virtual ~Mesh();
    
    // IResource interface
    bool load() override;
    void unload() override;
    
    // ========== Data Access ==========
    
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }
    
    size_t getVertexCount() const { return vertices.size(); }
    size_t getIndexCount() const { return indices.size(); }
    
    // ========== Data Modification ==========
    
    void setVertices(const std::vector<Vertex>& verts);
    void setIndices(const std::vector<uint32_t>& inds);
    
    void addVertex(const Vertex& vertex);
    void addTriangle(uint32_t i0, uint32_t i1, uint32_t i2);
    
    void clear();
    
    // ========== GPU Resources ==========
    
    /**
     * @brief Create GPU buffers from CPU data
     * @param device WebGPU device
     * @return true if successful
     */
    bool createGPUResources(wgpu::Device device);
    
    /**
     * @brief Release GPU buffers
     */
    void releaseGPUResources();
    
    wgpu::Buffer getVertexBuffer() const { return vertexBuffer; }
    wgpu::Buffer getIndexBuffer() const { return indexBuffer; }
    bool hasGPUResources() const { return gpuDataCreated; }
    
    // ========== Mesh Generation ==========
    
    /**
     * @brief Generate a cube mesh
     * @param size Cube size (default 1.0)
     */
    static Mesh* createCube(const std::string& name = "Cube", float size = 1.0f);
    
    /**
     * @brief Generate a sphere mesh
     * @param name Mesh name
     * @param radius Sphere radius
     * @param segments Number of segments (detail level)
     */
    static Mesh* createSphere(const std::string& name = "Sphere", 
                             float radius = 1.0f, 
                             int segments = 32);
    
    /**
     * @brief Generate a plane mesh
     * @param name Mesh name
     * @param width Plane width
     * @param height Plane height
     */
    static Mesh* createPlane(const std::string& name = "Plane",
                            float width = 1.0f,
                            float height = 1.0f);
    
    // ========== Utility ==========
    
    /**
     * @brief Calculate bounding box
     */
    void calculateBounds(Vec3& min, Vec3& max) const;
    
    /**
     * @brief Calculate normals from geometry
     */
    void calculateNormals();
};

} // namespace resource
} // namespace rs_engine
