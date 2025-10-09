#include "Mesh.h"
#include <cmath>
#include <iostream>

namespace rs_engine {
namespace resource {

Mesh::Mesh() {
    metadata.type = ResourceType::Mesh;
    metadata.state = ResourceState::Unloaded;
}

Mesh::Mesh(const std::string& name) : Mesh() {
    metadata.name = name;
}

Mesh::~Mesh() {
    unload();
}

bool Mesh::load() {
    if (metadata.state == ResourceState::Loaded) {
        return true;
    }
    
    // For procedural meshes, data is already in memory
    if (!vertices.empty()) {
        metadata.state = ResourceState::Loaded;
        metadata.memorySize = vertices.size() * sizeof(Vertex) + 
                             indices.size() * sizeof(uint32_t);
        return true;
    }
    
    // For file-based meshes, loading is done by ModelLoader
    metadata.state = ResourceState::Failed;
    return false;
}

void Mesh::unload() {
    releaseGPUResources();
    vertices.clear();
    indices.clear();
    metadata.state = ResourceState::Unloaded;
    metadata.memorySize = 0;
}

void Mesh::setVertices(const std::vector<Vertex>& verts) {
    vertices = verts;
    gpuDataCreated = false; // Need to recreate GPU resources
}

void Mesh::setIndices(const std::vector<uint32_t>& inds) {
    indices = inds;
    gpuDataCreated = false;
}

void Mesh::addVertex(const Vertex& vertex) {
    vertices.push_back(vertex);
    gpuDataCreated = false;
}

void Mesh::addTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
    indices.push_back(i0);
    indices.push_back(i1);
    indices.push_back(i2);
    gpuDataCreated = false;
}

void Mesh::clear() {
    vertices.clear();
    indices.clear();
    releaseGPUResources();
}

bool Mesh::createGPUResources(wgpu::Device device) {
    if (!device || vertices.empty()) {
        return false;
    }
    
    // Release old resources
    releaseGPUResources();
    
    // Create vertex buffer
    wgpu::BufferDescriptor vertexBufferDesc;
    vertexBufferDesc.size = vertices.size() * sizeof(Vertex);
    vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
    vertexBufferDesc.mappedAtCreation = false;
    
    vertexBuffer = device.CreateBuffer(&vertexBufferDesc);
    if (!vertexBuffer) {
        std::cerr << "Failed to create vertex buffer for mesh: " << metadata.name << std::endl;
        return false;
    }
    
    device.GetQueue().WriteBuffer(vertexBuffer, 0, vertices.data(), vertexBufferDesc.size);
    
    // Create index buffer (if indices exist)
    if (!indices.empty()) {
        wgpu::BufferDescriptor indexBufferDesc;
        indexBufferDesc.size = indices.size() * sizeof(uint32_t);
        indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBufferDesc.mappedAtCreation = false;
        
        indexBuffer = device.CreateBuffer(&indexBufferDesc);
        if (!indexBuffer) {
            std::cerr << "Failed to create index buffer for mesh: " << metadata.name << std::endl;
            vertexBuffer = nullptr;
            return false;
        }
        
        device.GetQueue().WriteBuffer(indexBuffer, 0, indices.data(), indexBufferDesc.size);
    }
    
    gpuDataCreated = true;
    return true;
}

void Mesh::releaseGPUResources() {
    if (vertexBuffer) {
        vertexBuffer.Destroy();
        vertexBuffer = nullptr;
    }
    if (indexBuffer) {
        indexBuffer.Destroy();
        indexBuffer = nullptr;
    }
    gpuDataCreated = false;
}

// ========== Mesh Generation ==========

Mesh* Mesh::createCube(const std::string& name, float size) {
    Mesh* mesh = new Mesh(name);
    
    float half = size * 0.5f;
    
    // Create vertices with proper normals for each face
    // Each face needs duplicate vertices because each vertex has a different normal per face
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Helper to add a quad face
    auto addQuad = [&](const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& v3, 
                       const Vec3& normal, const Vec3& color) {
        uint32_t baseIndex = static_cast<uint32_t>(vertices.size());
        
        vertices.emplace_back(v0, normal, Vec3(0, 0, 0), color);
        vertices.emplace_back(v1, normal, Vec3(1, 0, 0), color);
        vertices.emplace_back(v2, normal, Vec3(1, 1, 0), color);
        vertices.emplace_back(v3, normal, Vec3(0, 1, 0), color);
        
        // Two triangles (CCW winding)
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
    };
    
    // Front face (Z+)
    addQuad(
        Vec3(-half, -half,  half),  // bottom-left
        Vec3( half, -half,  half),  // bottom-right
        Vec3( half,  half,  half),  // top-right
        Vec3(-half,  half,  half),  // top-left
        Vec3(0, 0, 1),              // normal pointing forward
        Vec3(1, 0, 0)               // red
    );
    
    // Back face (Z-)
    addQuad(
        Vec3( half, -half, -half),  // bottom-right (flipped for CCW)
        Vec3(-half, -half, -half),  // bottom-left
        Vec3(-half,  half, -half),  // top-left
        Vec3( half,  half, -half),  // top-right
        Vec3(0, 0, -1),             // normal pointing backward
        Vec3(0, 1, 0)               // green
    );
    
    // Left face (X-)
    addQuad(
        Vec3(-half, -half, -half),  // bottom-back
        Vec3(-half, -half,  half),  // bottom-front
        Vec3(-half,  half,  half),  // top-front
        Vec3(-half,  half, -half),  // top-back
        Vec3(-1, 0, 0),             // normal pointing left
        Vec3(0, 0, 1)               // blue
    );
    
    // Right face (X+)
    addQuad(
        Vec3(half, -half,  half),   // bottom-front
        Vec3(half, -half, -half),   // bottom-back
        Vec3(half,  half, -half),   // top-back
        Vec3(half,  half,  half),   // top-front
        Vec3(1, 0, 0),              // normal pointing right
        Vec3(1, 1, 0)               // yellow
    );
    
    // Top face (Y+)
    addQuad(
        Vec3(-half, half,  half),   // front-left
        Vec3( half, half,  half),   // front-right
        Vec3( half, half, -half),   // back-right
        Vec3(-half, half, -half),   // back-left
        Vec3(0, 1, 0),              // normal pointing up
        Vec3(1, 0, 1)               // magenta
    );
    
    // Bottom face (Y-)
    addQuad(
        Vec3(-half, -half, -half),  // back-left
        Vec3( half, -half, -half),  // back-right
        Vec3( half, -half,  half),  // front-right
        Vec3(-half, -half,  half),  // front-left
        Vec3(0, -1, 0),             // normal pointing down
        Vec3(0, 1, 1)               // cyan
    );
    
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    mesh->load();
    
    return mesh;
}

Mesh* Mesh::createSphere(const std::string& name, float radius, int segments) {
    Mesh* mesh = new Mesh(name);
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Generate sphere vertices
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);
        
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * M_PI / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);
            
            Vec3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            Vec3 position = normal * radius;
            Vec3 texCoord(
                static_cast<float>(lon) / segments,
                static_cast<float>(lat) / segments,
                0.0f
            );
            
            vertices.emplace_back(position, normal, texCoord, Vec3(1, 1, 1));
        }
    }
    
    // Generate sphere indices
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            uint32_t first = lat * (segments + 1) + lon;
            uint32_t second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    mesh->load();
    
    return mesh;
}

Mesh* Mesh::createPlane(const std::string& name, float width, float height) {
    Mesh* mesh = new Mesh(name);

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    // Create plane at y=0 with 4 vertices
    // Note: We keep vertices at y=0, but bounds will be expanded slightly for picking
    std::vector<Vertex> vertices = {
        {{-halfW, 0, -halfH}, {0, 1, 0}, {0, 0, 0}, {1, 1, 1}},
        {{-halfW, 0,  halfH}, {0, 1, 0}, {0, 1, 0}, {1, 1, 1}},
        {{ halfW, 0,  halfH}, {0, 1, 0}, {1, 1, 0}, {1, 1, 1}},
        {{ halfW, 0, -halfH}, {0, 1, 0}, {1, 0, 0}, {1, 1, 1}},
    };

    std::vector<uint32_t> indices = {
        0, 1, 2,
        0, 2, 3
    };

    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    mesh->load();

    return mesh;
}

void Mesh::calculateBounds(Vec3& min, Vec3& max) const {
    if (vertices.empty()) {
        min = max = Vec3(0, 0, 0);
        return;
    }

    min = max = vertices[0].position;

    for (const auto& vertex : vertices) {
        min.x = std::min(min.x, vertex.position.x);
        min.y = std::min(min.y, vertex.position.y);
        min.z = std::min(min.z, vertex.position.z);

        max.x = std::max(max.x, vertex.position.x);
        max.y = std::max(max.y, vertex.position.y);
        max.z = std::max(max.z, vertex.position.z);
    }

    // Expand bounds by padding to prevent bounding box edges from overlapping with object surface
    // This padding is applied to ALL objects (not just flat ones) to create visual separation
    constexpr float BBOX_PADDING = 0.02f;  // Padding applied to all sides

    // First ensure minimum thickness for flat objects (like planes)
    constexpr float MIN_THICKNESS = 0.02f;
    if (max.x - min.x < MIN_THICKNESS) {
        float center = (min.x + max.x) * 0.5f;
        min.x = center - MIN_THICKNESS * 0.5f;
        max.x = center + MIN_THICKNESS * 0.5f;
    }
    if (max.y - min.y < MIN_THICKNESS) {
        float center = (min.y + max.y) * 0.5f;
        min.y = center - MIN_THICKNESS * 0.5f;
        max.y = center + MIN_THICKNESS * 0.5f;
    }
    if (max.z - min.z < MIN_THICKNESS) {
        float center = (min.z + max.z) * 0.5f;
        min.z = center - MIN_THICKNESS * 0.5f;
        max.z = center + MIN_THICKNESS * 0.5f;
    }

    // Then expand all bounds by padding (for all objects including cubes, spheres, etc.)
    min.x -= BBOX_PADDING;
    min.y -= BBOX_PADDING;
    min.z -= BBOX_PADDING;
    max.x += BBOX_PADDING;
    max.y += BBOX_PADDING;
    max.z += BBOX_PADDING;
}

void Mesh::calculateNormals() {
    if (indices.empty() || vertices.empty()) {
        return;
    }
    
    // Reset all normals
    for (auto& vertex : vertices) {
        vertex.normal = Vec3(0, 0, 0);
    }
    
    // Calculate face normals and accumulate
    for (size_t i = 0; i < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];
        
        Vec3 v0 = vertices[i0].position;
        Vec3 v1 = vertices[i1].position;
        Vec3 v2 = vertices[i2].position;
        
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 normal = edge1.cross(edge2);
        
        vertices[i0].normal = vertices[i0].normal + normal;
        vertices[i1].normal = vertices[i1].normal + normal;
        vertices[i2].normal = vertices[i2].normal + normal;
    }
    
    // Normalize all normals
    for (auto& vertex : vertices) {
        vertex.normal = vertex.normal.normalize();
    }
    
    gpuDataCreated = false; // Need to update GPU buffers
}

} // namespace resource
} // namespace rs_engine
