#pragma once

#include "Vec3.h"
#include <algorithm>
#include <limits>

namespace rs_engine {

/**
 * @brief Ray for raycasting and object picking
 * 
 * Used for:
 * - Mouse picking (screen to world)
 * - Collision detection
 * - Line of sight checks
 * 
 * Platform Support: 100% shared
 */
class Ray {
public:
    Vec3 origin;
    Vec3 direction;  // Should be normalized

    Ray() = default;
    Ray(const Vec3& origin, const Vec3& direction)
        : origin(origin), direction(direction) {}

    /**
     * @brief Get point along ray at distance t
     */
    Vec3 at(float t) const {
        return origin + direction * t;
    }

    /**
     * @brief Test intersection with AABB (Axis-Aligned Bounding Box)
     * 
     * Uses slab method for efficient ray-box intersection
     * 
     * @param min Minimum corner of AABB in world space
     * @param max Maximum corner of AABB in world space
     * @param tMin Output: minimum intersection distance (entry point)
     * @param tMax Output: maximum intersection distance (exit point)
     * @return true if ray intersects AABB
     */
    bool intersectAABB(const Vec3& min, const Vec3& max, float& tMin, float& tMax) const {
        tMin = -std::numeric_limits<float>::infinity();
        tMax = std::numeric_limits<float>::infinity();

        // Test intersection with each axis-aligned slab
        for (int i = 0; i < 3; i++) {
            float originComp = (i == 0) ? origin.x : (i == 1) ? origin.y : origin.z;
            float dirComp = (i == 0) ? direction.x : (i == 1) ? direction.y : direction.z;
            float minComp = (i == 0) ? min.x : (i == 1) ? min.y : min.z;
            float maxComp = (i == 0) ? max.x : (i == 1) ? max.y : max.z;

            if (std::abs(dirComp) < 1e-8f) {
                // Ray is parallel to slab. No hit if origin not within slab
                if (originComp < minComp || originComp > maxComp) {
                    return false;
                }
            } else {
                float invD = 1.0f / dirComp;
                float t0 = (minComp - originComp) * invD;
                float t1 = (maxComp - originComp) * invD;

                if (invD < 0.0f) {
                    std::swap(t0, t1);
                }

                tMin = t0 > tMin ? t0 : tMin;
                tMax = t1 < tMax ? t1 : tMax;

                if (tMax <= tMin) {
                    return false;
                }
            }
        }

        return true;
    }
    
    /**
     * @brief Test intersection with triangle using Möller-Trumbore algorithm
     * 
     * Fast ray-triangle intersection test with barycentric coordinates
     * 
     * @param v0 First vertex of triangle
     * @param v1 Second vertex of triangle
     * @param v2 Third vertex of triangle
     * @param t Output: distance along ray to intersection point
     * @param u Output: barycentric coordinate u (optional)
     * @param v Output: barycentric coordinate v (optional)
     * @return true if ray intersects triangle
     */
    bool intersectTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, 
                          float& t, float* u = nullptr, float* v_out = nullptr) const {
        constexpr float EPSILON = 1e-8f;
        
        // Find vectors for two edges sharing v0
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        
        // Begin calculating determinant - also used to calculate u parameter
        Vec3 pvec = direction.cross(edge2);
        
        // If determinant is near zero, ray lies in plane of triangle
        float det = edge1.dot(pvec);
        
        // No culling: accept both front and back facing triangles
        if (std::abs(det) < EPSILON) {
            return false;
        }
        
        float invDet = 1.0f / det;
        
        // Calculate distance from v0 to ray origin
        Vec3 tvec = origin - v0;
        
        // Calculate u parameter and test bounds
        float u_val = tvec.dot(pvec) * invDet;
        if (u_val < 0.0f || u_val > 1.0f) {
            return false;
        }
        
        // Prepare to test v parameter
        Vec3 qvec = tvec.cross(edge1);
        
        // Calculate v parameter and test bounds
        float v_val = direction.dot(qvec) * invDet;
        if (v_val < 0.0f || u_val + v_val > 1.0f) {
            return false;
        }
        
        // Calculate t (distance along ray)
        t = edge2.dot(qvec) * invDet;
        
        // Only accept intersections in front of ray origin
        if (t < 0.0f) {
            return false;
        }
        
        // Output barycentric coordinates if requested
        if (u) *u = u_val;
        if (v_out) *v_out = v_val;
        
        return true;
    }
};

} // namespace rs_engine
