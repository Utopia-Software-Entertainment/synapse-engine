#pragma once

#include <core/Math/Math.h>
#include <glm/glm.hpp>

namespace synapse {

// SIMD-capable vector types (GLM: SSE/NEON intrinsics on -march host flags).
using Vec2  = glm::vec2;
using Vec3  = glm::vec3;
using Vec4  = glm::vec4;
using IVec2 = glm::ivec2;
using IVec3 = glm::ivec3;
using IVec4 = glm::ivec4;

inline f32 Length(const Vec3& v)            { return glm::length(v); }
inline f32 LengthSquared(const Vec3& v)     { return glm::dot(v, v); }
inline f32 Distance(const Vec3& a, const Vec3& b) { return glm::distance(a, b); }
inline f32 Dot(const Vec3& a, const Vec3& b)       { return glm::dot(a, b); }
inline Vec3 Cross(const Vec3& a, const Vec3& b)    { return glm::cross(a, b); }

inline Vec3 Normalize(const Vec3& v)
{
    return glm::normalize(v);
}

// Returns zero for a zero-length vector (glm::normalize would give NaN).
inline Vec3 SafeNormalize(const Vec3& v)
{
    const f32 lenSq = LengthSquared(v);
    if (lenSq < math::kEpsilon)
    {
        return Vec3(0.0f);
    }
    return v * (1.0f / std::sqrt(lenSq));
}

inline Vec3 Reflect(const Vec3& v, const Vec3& n)
{
    return v - n * (2.0f * Dot(v, n));
}

} // namespace synapse