#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <physics/Jolt/PhysicsWorld.h>
#include <physics/Jolt/RigidBody.h>

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <limits>

using namespace synapse::physics;

namespace {

constexpr float kDt = 1.0f / 60.0f;
constexpr float kRestY = -0.5f;

void Step(PhysicsWorld& world, int frames)
{
    for (int i = 0; i < frames; ++i)
    {
        world.Step(kDt);
    }
}

RigidBody MakeFloor(PhysicsWorld& world)
{
    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    return RigidBody(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                     MotionType::Static);
}

RigidBody MakeCube(PhysicsWorld& world, const glm::vec3& position)
{
    return RigidBody(world, ShapeDesc{}, position);
}

} // namespace

TEST_CASE("Physics: un corps tombe sous gravite")
{
    PhysicsWorld world;
    const auto floor = MakeFloor(world);
    auto cube = MakeCube(world, glm::vec3(0.0f, 5.0f, 0.0f));

    Step(world, 10);
    const float y = cube.GetPosition().y;
    CHECK(y < 5.0f);
    CHECK(y > 1.0f);
}

TEST_CASE("Physics: un corps se repose sur le sol")
{
    PhysicsWorld world;
    const auto floor = MakeFloor(world);
    auto cube = MakeCube(world, glm::vec3(0.0f, 5.0f, 0.0f));

    Step(world, 360);
    const float y = cube.GetPosition().y;
    CHECK(y == doctest::Approx(kRestY).epsilon(0.05));
}

TEST_CASE("Physics: une sphere rebondit")
{
    PhysicsWorld world;
    const auto floor = MakeFloor(world);

    ShapeDesc sphereShape;
    sphereShape.kind = ShapeKind::Sphere;
    sphereShape.radius = 0.5f;
    RigidBody sphere(world, sphereShape, glm::vec3(0.0f, 4.0f, 0.0f),
                     glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Dynamic, 0.5f, 0.6f);

    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    for (int i = 0; i < 120; ++i)
    {
        world.Step(1.0f / 60.0f);
        const float y = sphere.GetPosition().y;
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }
    CHECK(minY < -0.35f);
    CHECK(maxY > 0.5f);

    Step(world, 600);
    CHECK(sphere.GetPosition().y == doctest::Approx(kRestY).epsilon(0.05));
}

TEST_CASE("Physics: deux cubes empiles")
{
    PhysicsWorld world;
    const auto floor = MakeFloor(world);
    auto bottom = MakeCube(world, glm::vec3(0.0f, 0.0f, 0.0f));
    auto top = MakeCube(world, glm::vec3(0.0f, 2.0f, 0.0f));

    Step(world, 600);
    CHECK(bottom.GetPosition().y == doctest::Approx(kRestY).epsilon(0.05));
    CHECK(top.GetPosition().y == doctest::Approx(kRestY + 1.0f).epsilon(0.05));
}

TEST_CASE("Physics: un corps statique ne bouge pas")
{
    PhysicsWorld world;
    ShapeDesc staticShape;
    staticShape.kind = ShapeKind::Box;
    staticShape.halfExtent = glm::vec3(1.0f);
    RigidBody block(world, staticShape, glm::vec3(2.0f, 0.0f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    Step(world, 60);
    const glm::vec3 pos = block.GetPosition();
    CHECK(pos.x == doctest::Approx(2.0f).epsilon(0.001));
    CHECK(pos.y == doctest::Approx(0.0f).epsilon(0.001));
    CHECK(pos.z == doctest::Approx(0.0f).epsilon(0.001));
}

TEST_CASE("Physics: la rotation est uniforme pendant la chute")
{
    PhysicsWorld world;
    const auto floor = MakeFloor(world);
    auto cube = MakeCube(world, glm::vec3(0.0f, 3.0f, 0.0f));

    Step(world, 60);
    const glm::quat rot = cube.GetRotation();
    const float dot = glm::dot(rot, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    CHECK(dot > 0.99f);
}
