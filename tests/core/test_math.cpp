#include <doctest/doctest.h>

#include <core/Math/Frustum.h>
#include <core/Math/Matrix.h>
#include <core/Math/Quaternion.h>
#include <core/Math/Ray.h>
#include <core/Math/Vector.h>

using namespace synapse;
using namespace synapse::math;

TEST_CASE("Math: Radians/Degrees roundtrip")
{
    CHECK(NearlyEqual(Degrees(Radians(90.0f)), 90.0f));
    CHECK(NearlyEqual(Radians(Degrees(1.5f)), 1.5f));
}

TEST_CASE("Math: Clamp/Lerp")
{
    CHECK(Clamp(5.0f, 0.0f, 1.0f) == 1.0f);
    CHECK(Clamp(-1.0f, 0.0f, 1.0f) == 0.0f);
    CHECK(NearlyEqual(Lerp(0.0f, 10.0f, 0.5f), 5.0f));
    CHECK(SmoothStep(0.0f, 1.0f, 0.5f) == doctest::Approx(0.5f));
}

TEST_CASE("Vector: dot/cross/normalize")
{
    const Vec3 a(1, 0, 0);
    const Vec3 b(0, 1, 0);
    CHECK(Dot(a, b) == 0.0f);
    CHECK(Cross(a, b) == Vec3(0, 0, 1));

    const Vec3 n = Normalize(Vec3(3, 4, 0));
    CHECK(NearlyEqual(Length(n), 1.0f));
    CHECK(SafeNormalize(Vec3(0.0f)) == Vec3(0.0f));

    const Vec3 r = Reflect(a, b);
    CHECK(NearlyEqual(Dot(r, b), -Dot(a, b)));
}

TEST_CASE("Matrix: transform point")
{
    const Mat4 t = Translation(Vec3(1, 2, 3));
    const Vec4 p = TransformPoint(t, Vec4(1, 0, 0, 1));
    CHECK(Vec3(p) == Vec3(2, 2, 3));

    const Mat4 s = Scale(Vec3(2, 2, 2));
    CHECK(TransformPoint(s, Vec4(1, 0, 0, 1)) == Vec4(2, 0, 0, 1));

    const Mat4 inv = Inverse(Translation(Vec3(1, 2, 3)));
    CHECK(TransformPoint(inv, Vec4(2, 2, 3, 1)) == Vec4(1, 0, 0, 1));
}

TEST_CASE("Matrix: lookAt camera")
{
    const Mat4 view = LookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
    // Camera forward is world -Z; in view space the camera looks down its own -Z.
    CHECK(TransformDirection(view, Vec3(0, 0, -1)) == Vec3(0, 0, -1));
    CHECK(TransformDirection(view, Vec3(0, 1, 0)).y == doctest::Approx(1.0f));
    // Origin sits 5 units in front of the camera → view-space z = -5.
    CHECK(TransformPoint(view, Vec4(0, 0, 0, 1)) == Vec4(0, 0, -5, 1));
}

TEST_CASE("Quaternion: euler roundtrip")
{
    const Quat q = FromEuler(0.3f, -0.6f, 0.9f);
    const Vec3 e = ToEuler(q);
    CHECK(NearlyEqual(e.x, 0.3f, 1e-4f));
    CHECK(NearlyEqual(e.y, -0.6f, 1e-4f));
    CHECK(NearlyEqual(e.z, 0.9f, 1e-4f));
}

TEST_CASE("Quaternion: axis-angle rotation")
{
    const Quat q = FromAxisAngle(Vec3(0, 1, 0), kHalfPi);
    const Vec3 v = Rotate(q, Vec3(1, 0, 0));
    CHECK(NearlyEqual(v.x, 0.0f, 1e-5f));
    CHECK(NearlyEqual(v.z, -1.0f, 1e-5f));
}

TEST_CASE("Quaternion: slerp endpoints")
{
    const Quat a = IdentityQuat();
    const Quat b = FromAxisAngle(Vec3(0, 1, 0), kHalfPi);
    CHECK(Slerp(a, b, 0.0f) == a);
    CHECK(Slerp(a, b, 1.0f) == b);
}

TEST_CASE("Ray: closest point and intersections")
{
    const Ray ray(Vec3(0, 0, 0), Vec3(0, 0, 1));

    CHECK(ClosestPointOnRay(ray, Vec3(5, 5, 5)) == Vec3(0, 0, 5));
    CHECK(ClosestPointOnRay(ray, Vec3(5, 5, -5)) == Vec3(0, 0, 0));

    f32 t = 0.0f;
    CHECK(IntersectPlane(ray, Vec3(0, 0, 3), Vec3(0, 0, 1), t));
    CHECK(NearlyEqual(t, 3.0f));

    CHECK(IntersectSphere(ray, Vec3(0, 0, 5), 1.0f, t));
    CHECK(NearlyEqual(t, 4.0f));
    CHECK_FALSE(IntersectSphere(ray, Vec3(3, 0, 5), 1.0f, t));

    CHECK(IntersectAABB(ray, Vec3(-1, -1, 4), Vec3(1, 1, 6), t));
    CHECK(NearlyEqual(t, 4.0f));
    CHECK_FALSE(IntersectAABB(ray, Vec3(2, 2, 4), Vec3(3, 3, 6), t));

    // Parallel ray misses the plane.
    const Ray parallel(Vec3(0, 0, 0), Vec3(1, 0, 0));
    CHECK_FALSE(IntersectPlane(parallel, Vec3(0, 0, 3), Vec3(0, 0, 1), t));
}

TEST_CASE("Frustum: from view-projection, point and bounds tests")
{
    const Mat4 proj = Perspective(Radians(60.0f), 16.0f / 9.0f, 0.1f, 100.0f);
    const Mat4 view = LookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
    const Frustum f = Frustum::FromViewProj(proj * view);

    CHECK(f.ContainsPoint(Vec3(0, 0, 0)));
    CHECK_FALSE(f.ContainsPoint(Vec3(0, 0, 200)));      // beyond far plane
    CHECK_FALSE(f.ContainsPoint(Vec3(0, 50, 0)));       // above frustum

    CHECK(f.IntersectsSphere(Vec3(0, 0, -1), 0.5f));
    CHECK_FALSE(f.IntersectsSphere(Vec3(0, 0, -200), 1.0f));

    CHECK(f.IntersectsAABB(Vec3(-1, -1, -1), Vec3(1, 1, 1)));
    CHECK_FALSE(f.IntersectsAABB(Vec3(50, 50, 50), Vec3(51, 51, 51)));
}