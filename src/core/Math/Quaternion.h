#pragma once

#include <core/Math/Vector.h>
#include <glm/gtc/quaternion.hpp>

namespace synapse {

using Quat = glm::quat;

inline Quat IdentityQuat()                        { return Quat(1.0f, 0.0f, 0.0f, 0.0f); }

// Euler order: pitch (X), yaw (Y), roll (Z) — matching typical game conventions.
inline Quat FromEuler(f32 pitch, f32 yaw, f32 roll)
{
    return Quat(Vec3(pitch, yaw, roll));
}

inline Vec3 ToEuler(const Quat& q)
{
    return glm::eulerAngles(q);
}

inline Quat FromAxisAngle(const Vec3& axis, f32 angleRad)
{
    return glm::angleAxis(angleRad, axis);
}

inline Quat NormalizeQuat(const Quat& q)          { return glm::normalize(q); }
inline Quat InverseQuat(const Quat& q)            { return glm::inverse(q); }
inline Quat Slerp(const Quat& a, const Quat& b, f32 t) { return glm::slerp(a, b, t); }

// Rotate a vector by a quaternion (q is assumed normalized).
inline Vec3 Rotate(const Quat& q, const Vec3& v)  { return q * v; }

} // namespace synapse