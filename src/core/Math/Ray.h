#pragma once

#include <core/Math/Vector.h>
#include <limits>

namespace synapse {

// Ray with a normalized direction. `direction` is re-normalized at construction.
struct Ray
{
    Vec3 origin{0.0f};
    Vec3 direction{0.0f, 0.0f, -1.0f};

    Ray() = default;

    Ray(const Vec3& o, const Vec3& d)
        : origin(o), direction(SafeNormalize(d)) {}

    Vec3 At(f32 t) const { return origin + direction * t; }
};

// Closest point on the ray to `point` (t clamped to >= 0).
inline Vec3 ClosestPointOnRay(const Ray& ray, const Vec3& point)
{
    const f32 t = math::Max(Dot(point - ray.origin, ray.direction), 0.0f);
    return ray.At(t);
}

// Ray vs infinite plane. Returns false if parallel or if hit is behind origin.
inline bool IntersectPlane(const Ray& ray, const Vec3& planePoint, const Vec3& planeNormal, f32& outT)
{
    const f32 denom = Dot(ray.direction, planeNormal);
    if (math::NearlyEqual(denom, 0.0f))
    {
        return false;
    }
    const f32 t = Dot(planePoint - ray.origin, planeNormal) / denom;
    if (t < 0.0f)
    {
        return false;
    }
    outT = t;
    return true;
}

// Ray vs sphere (both intersections allowed; returns nearest).
inline bool IntersectSphere(const Ray& ray, const Vec3& center, f32 radius, f32& outT)
{
    const Vec3 oc = ray.origin - center;
    const f32 b = Dot(oc, ray.direction);
    const f32 c = Dot(oc, oc) - radius * radius;
    const f32 disc = b * b - c;
    if (disc < 0.0f)
    {
        return false;
    }
    f32 t = -b - std::sqrt(disc);
    if (t < 0.0f)
    {
        t = -b + std::sqrt(disc);
    }
    if (t < 0.0f)
    {
        return false;
    }
    outT = t;
    return true;
}

// Ray vs AABB (slab method, handles zero direction via infinities).
inline bool IntersectAABB(const Ray& ray, const Vec3& min, const Vec3& max, f32& outT)
{
    f32 tMin = 0.0f;
    f32 tMax = std::numeric_limits<f32>::max();

    for (u32 i = 0; i < 3; ++i)
    {
        const f32 invD = 1.0f / ray.direction[i];
        f32 t1 = (min[i] - ray.origin[i]) * invD;
        f32 t2 = (max[i] - ray.origin[i]) * invD;
        if (t1 > t2)
        {
            std::swap(t1, t2);
        }
        tMin = math::Max(tMin, t1);
        tMax = math::Min(tMax, t2);
        if (tMin > tMax)
        {
            return false;
        }
    }
    outT = tMin;
    return true;
}

} // namespace synapse