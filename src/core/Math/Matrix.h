#pragma once

#include <core/Math/Quaternion.h>
#include <glm/gtc/matrix_transform.hpp>

namespace synapse {

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

inline Mat4 Identity4()               { return glm::mat4(1.0f); }
inline Mat3 Identity3()               { return glm::mat3(1.0f); }

inline Mat4 Translation(const Vec3& t) { return glm::translate(Identity4(), t); }
inline Mat4 Scale(const Vec3& s)       { return glm::scale(Identity4(), s); }

inline Mat4 Rotation(const Quat& q)    { return glm::mat4_cast(q); }

inline Mat4 RotationX(f32 angleRad)    { return glm::rotate(Identity4(), angleRad, Vec3(1, 0, 0)); }
inline Mat4 RotationY(f32 angleRad)    { return glm::rotate(Identity4(), angleRad, Vec3(0, 1, 0)); }
inline Mat4 RotationZ(f32 angleRad)    { return glm::rotate(Identity4(), angleRad, Vec3(0, 0, 1)); }

inline Mat4 Perspective(f32 fovyRad, f32 aspect, f32 zNear, f32 zFar)
{
    return glm::perspective(fovyRad, aspect, zNear, zFar);
}

inline Mat4 Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
{
    return glm::ortho(left, right, bottom, top, zNear, zFar);
}

inline Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
{
    return glm::lookAt(eye, center, up);
}

inline Mat4 Inverse(const Mat4& m)     { return glm::inverse(m); }
inline Mat4 Transpose(const Mat4& m)   { return glm::transpose(m); }

inline Vec4 TransformPoint(const Mat4& m, const Vec4& p) { return m * p; }
inline Vec3 TransformDirection(const Mat4& m, const Vec3& v) { return glm::mat3(m) * v; }

} // namespace synapse