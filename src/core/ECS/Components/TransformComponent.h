#pragma once

#include <core/ECS/Entity.h>
#include <core/Math/Matrix.h>
#include <core/Math/Quaternion.h>
#include <core/Math/Vector.h>

namespace synapse::ecs {

// TRS transform. Used by nearly every gameplay entity; kept cache-friendly
// (position/rotation/scale contiguous) for SIMD-friendly DOD iteration.
struct TransformComponent
{
    Vec3 position{0.0f};
    Quat rotation = IdentityQuat();
    Vec3 scale{1.0f};

    Mat4 LocalMatrix() const
    {
        return Translation(position) * Rotation(rotation) * Scale(scale);
    }

    // Local axes (rotation applied to the identity frame).
    Vec3 Forward() const { return Rotate(rotation, Vec3(0.0f, 0.0f, -1.0f)); }
    Vec3 Right()   const { return Rotate(rotation, Vec3(1.0f, 0.0f, 0.0f)); }
    Vec3 Up()      const { return Rotate(rotation, Vec3(0.0f, 1.0f, 0.0f)); }
};

} // namespace synapse::ecs