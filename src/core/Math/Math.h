#pragma once

#include <core/Types.h>
#include <cmath>
#include <type_traits>

namespace synapse::math {

constexpr f32 kPi       = 3.14159265358979323846f;
constexpr f32 kTwoPi    = 6.28318530717958647692f;
constexpr f32 kHalfPi   = 1.57079632679489661923f;
constexpr f32 kDegToRad = kPi / 180.0f;
constexpr f32 kRadToDeg = 180.0f / kPi;
constexpr f32 kEpsilon  = 1e-6f;

template <typename T>
constexpr T Clamp(T v, T lo, T hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

template <typename T>
constexpr T Lerp(T a, T b, f32 t)
{
    return static_cast<T>(a + (b - a) * t);
}

template <typename T>
constexpr T SmoothStep(T edge0, T edge1, T x)
{
    const T t = Clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

constexpr f32 Radians(f32 degrees) { return degrees * kDegToRad; }
constexpr f32 Degrees(f32 radians) { return radians * kRadToDeg; }

template <typename T>
bool NearlyEqual(T a, T b, T epsilon = static_cast<T>(kEpsilon))
{
    return std::abs(a - b) <= epsilon;
}

template <typename T>
constexpr T Min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
constexpr T Max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T Abs(T v)
{
    return v < T(0) ? -v : v;
}

static_assert(std::is_same_v<f32, float>, "synapse::f32 must be float for SIMD math.");

} // namespace synapse::math