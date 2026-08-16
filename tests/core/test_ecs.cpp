#include <doctest/doctest.h>

#include <core/ECS/Component.h>
#include <core/ECS/Entity.h>
#include <core/ECS/Registry.h>
#include <core/ECS/Components/TransformComponent.h>
#include <core/Math/Matrix.h>
#include <vector>
#include <algorithm>

using namespace synapse;
using namespace synapse::ecs;
using namespace synapse::math;

namespace {

struct Position
{
    Vec3 value{0.0f};
};

struct Velocity
{
    Vec3 value{0.0f};
};

} // namespace

TEST_CASE("ECS: spawn/despawn lifecycle")
{
    Registry registry;
    const Entity e = Spawn(registry);
    CHECK(IsValid(e));
    CHECK(ToIndex(e) != ToIndex(kNullEntity));

    Despawn(registry, e);
    // Slot may be reused; EnTT keeps a version counter to invalidate stale refs.
    CHECK_FALSE(registry.valid(e));
    CHECK_FALSE(registry.storage<Position>().contains(e));
}

TEST_CASE("ECS: component add/get/has/remove")
{
    Registry registry;
    const Entity e = Spawn(registry);

    AddComponent<Position>(registry, e, Vec3(1, 2, 3));
    CHECK(HasComponent<Position>(registry, e));
    CHECK_FALSE(HasComponent<Velocity>(registry, e));

    GetComponent<Position>(registry, e).value += Vec3(1, 0, 0);
    CHECK(GetComponent<Position>(registry, e).value == Vec3(2, 2, 3));

    RemoveComponent<Position>(registry, e);
    CHECK_FALSE(HasComponent<Position>(registry, e));
}

TEST_CASE("ECS: view iterates matching entities")
{
    Registry registry;

    for (u32 i = 0; i < 10; ++i)
    {
        const Entity e = Spawn(registry);
        AddComponent<Position>(registry, e, Vec3(static_cast<f32>(i), 0, 0));
        if (i % 2 == 0)
        {
            AddComponent<Velocity>(registry, e, Vec3(1, 0, 0));
        }
    }

    // Iteration order is NOT guaranteed by EnTT — collect and sort.
    std::vector<f32> xs;
    for (auto [entity, pos, vel] : View<Position, Velocity>(registry).each())
    {
        (void)entity;
        (void)vel;
        xs.push_back(pos.value.x);
    }
    std::sort(xs.begin(), xs.end());
    CHECK(xs == std::vector<f32>{0.0f, 2.0f, 4.0f, 6.0f, 8.0f});
}

TEST_CASE("ECS: component ids are unique per type")
{
    CHECK(ComponentId<Position>() != ComponentId<Velocity>());
    CHECK(ComponentId<Position>() == ComponentId<Position>());
}

TEST_CASE("ECS: transform component")
{
    TransformComponent tc;
    CHECK(tc.scale == Vec3(1.0f));
    CHECK(tc.Forward() == Vec3(0, 0, -1));
    CHECK(tc.Right() == Vec3(1, 0, 0));
    CHECK(tc.Up() == Vec3(0, 1, 0));

    tc.position = Vec3(10, 0, 0);
    const Vec4 origin = TransformPoint(tc.LocalMatrix(), Vec4(0, 0, 0, 1));
    CHECK(Vec3(origin) == Vec3(10, 0, 0));

    tc.rotation = FromEuler(0.0f, math::kHalfPi, 0.0f);   // yaw 90° → forward = -X
    const Vec3 fwd = tc.Forward();
    CHECK(NearlyEqual(fwd.x, -1.0f, 1e-4f));
    CHECK(NearlyEqual(fwd.z, 0.0f, 1e-4f));
}