#pragma once

#include <core/Math/Matrix.h>
#include <core/Math/Vector.h>

namespace synapse {

// View frustum as 6 planes (Gribb-Hartmann extraction from a view-projection
// matrix). Plane normals point INWARD; a point is inside iff plane distance >= 0.
struct Frustum
{
    enum PlaneIndex : u32
    {
        Left   = 0,
        Right  = 1,
        Bottom = 2,
        Top    = 3,
        Near   = 4,
        Far    = 5,
        Count  = 6
    };

    Vec4 planes[Count]{};

    static Frustum FromViewProj(const Mat4& viewProj)
    {
        Frustum f;
        for (u32 i = 0; i < Count; ++i)
        {
            const u32 row = i % 3;
            const Vec4 r = Vec4(viewProj[0][row], viewProj[1][row], viewProj[2][row], viewProj[3][row]);
            const Vec4 last = Vec4(viewProj[0][3], viewProj[1][3], viewProj[2][3], viewProj[3][3]);
            const Vec4 plane = (i % 2 == 0) ? (last + r) : (last - r);
            const Vec3 normal(plane.x, plane.y, plane.z);
            const f32 len = Length(normal);
            f.planes[i] = plane / len;
        }
        return f;
    }

    f32 PlaneDistance(u32 index, const Vec3& point) const
    {
        const Vec4& p = planes[index];
        return p.x * point.x + p.y * point.y + p.z * point.z + p.w;
    }

    bool ContainsPoint(const Vec3& point) const
    {
        for (u32 i = 0; i < Count; ++i)
        {
            if (PlaneDistance(i, point) < 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    bool IntersectsSphere(const Vec3& center, f32 radius) const
    {
        for (u32 i = 0; i < Count; ++i)
        {
            if (PlaneDistance(i, center) < -radius)
            {
                return false;
            }
        }
        return true;
    }

    bool IntersectsAABB(const Vec3& min, const Vec3& max) const
    {
        for (u32 i = 0; i < Count; ++i)
        {
            const Vec4& p = planes[i];
            const Vec3 posVertex(p.x < 0.0f ? min.x : max.x,
                                 p.y < 0.0f ? min.y : max.y,
                                 p.z < 0.0f ? min.z : max.z);
            if (PlaneDistance(i, posVertex) < 0.0f)
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace synapse