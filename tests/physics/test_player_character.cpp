#include <doctest/doctest.h>

#include <physics/Jolt/PhysicsWorld.h>
#include <physics/Jolt/RigidBody.h>
#include <physics/Jolt/PlayerCharacter.h>

#include <glm/gtc/quaternion.hpp>

using namespace synapse::physics;

namespace {

constexpr float kDt = 1.0f / 60.0f;

} // namespace

TEST_CASE("Physics: le personnage tombe et se pose sur le sol")
{
    PhysicsWorld world;

    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    RigidBody floor(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    PlayerCharacter player(world, glm::vec3(0.0f, 3.0f, 0.0f));

    for (int i = 0; i < 240; ++i)
    {
        player.Update(kDt, glm::vec3(0.0f), false);
    }

    CHECK(player.IsOnGround());
    // Sol à y=-1.0 ; capsule (rayon 0.4, demi-hauteur 0.8) → centre à -1.0 + 1.2 = 0.2.
    CHECK(player.GetPosition().y == doctest::Approx(0.2f).epsilon(0.1));
}

TEST_CASE("Physics: le personnage ne traverse pas un mur")
{
    PhysicsWorld world;

    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    RigidBody floor(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    ShapeDesc wallShape;
    wallShape.kind = ShapeKind::Box;
    wallShape.halfExtent = glm::vec3(0.1f, 1.0f, 3.0f);
    RigidBody wall(world, wallShape, glm::vec3(2.0f, 0.5f, 0.0f),
                   glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    PlayerCharacter player(world, glm::vec3(0.0f, 0.0f, 0.0f));

    // Laisse le personnage se poser au sol d'abord.
    for (int i = 0; i < 120; ++i)
    {
        player.Update(kDt, glm::vec3(0.0f), false);
    }
    CHECK(player.IsOnGround());

    // Marche vers le mur pendant 3 secondes (bien plus que nécessaire).
    for (int i = 0; i < 180; ++i)
    {
        player.Update(kDt, glm::vec3(1.0f, 0.0f, 0.0f), false);
    }

    // Le mur commence à x=1.9 (2.0 - 0.1). Le personnage doit rester bloqué avant.
    const float x = player.GetPosition().x;
    CHECK(x < 1.9f);
}

TEST_CASE("Physics: le personnage ne traverse pas un cube dynamique au sol")
{
    PhysicsWorld world;

    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    RigidBody floor(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    RigidBody cube(world, ShapeDesc{}, glm::vec3(1.5f, 0.5f, 0.0f));

    PlayerCharacter player(world, glm::vec3(0.0f, 0.0f, 0.0f));

    for (int i = 0; i < 120; ++i)
    {
        player.Update(kDt, glm::vec3(0.0f), false);
    }

    for (int i = 0; i < 180; ++i)
    {
        player.Update(kDt, glm::vec3(1.0f, 0.0f, 0.0f), false);
    }

    // Le cube occupe x ∈ [1.0, 2.0] (centré 1.5, demi-taille 0.5) ;
    // le personnage (rayon 0.4) ne peut pas le franchir.
    CHECK(player.GetPosition().x < 1.5f);
}
TEST_CASE("Physics: le personnage au repos ne derive pas")
{
    PhysicsWorld world;

    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    RigidBody floor(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    PlayerCharacter player(world, glm::vec3(0.0f, 0.2f, 3.0f));

    for (int i = 0; i < 600; ++i)
    {
        player.Update(kDt, glm::vec3(0.0f), false);
    }

    const glm::vec3 p = player.GetPosition();
    CHECK(p.x == doctest::Approx(0.0f).epsilon(0.01f));
    CHECK(p.z == doctest::Approx(3.0f).epsilon(0.01f));
}

TEST_CASE("Physics: pas de derive avec la scene complete (corps dynamiques)")
{
    PhysicsWorld world;

    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    RigidBody floor(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    RigidBody cube(world, ShapeDesc{}, glm::vec3(0.0f, 0.5f, 0.0f));

    ShapeDesc sphereShape;
    sphereShape.kind = ShapeKind::Sphere;
    sphereShape.radius = 0.5f;
    RigidBody sphere(world, sphereShape, glm::vec3(-1.5f, 2.0f, 0.0f),
                     glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Dynamic, 0.5f, 0.35f);
    sphere.SetLinearVelocity(glm::vec3(2.2f, 0.0f, 0.0f));

    for (int i = 0; i < 6; ++i)
    {
        const float angle = static_cast<float>(i) * glm::two_pi<float>() / 6.0f;
        RigidBody orb(world, ShapeDesc{}, glm::vec3(std::cos(angle) * 2.2f, 2.0f + i * 0.5f,
                                                    std::sin(angle) * 2.2f));
        orb.SetLinearVelocity(glm::vec3(-std::sin(angle) * 1.5f, 0.0f, std::cos(angle) * 1.5f));
    }

    PlayerCharacter player(world, glm::vec3(0.0f, 0.2f, 3.0f));

    for (int i = 0; i < 450; ++i)
    {
        world.Step(kDt);
        player.Update(kDt, glm::vec3(0.0f), false);
    }

    const glm::vec3 p = player.GetPosition();
    CHECK(p.x == doctest::Approx(0.0f).epsilon(0.05f));
    CHECK(p.z == doctest::Approx(3.0f).epsilon(0.05f));
}

TEST_CASE("Physics: derive a petit dt (spawn au contact exact, 1000 fps)")
{
    PhysicsWorld world;

    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    RigidBody floor(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    PlayerCharacter player(world, glm::vec3(0.0f, 0.2f, 3.0f));

    const float kSmallDt = 0.0009f;
    for (int i = 0; i < 1200; ++i)
        player.Update(kSmallDt, glm::vec3(0.0f), false);

    const glm::vec3 p = player.GetPosition();
    CHECK(p.x == doctest::Approx(0.0f).epsilon(0.05f));
    CHECK(p.z == doctest::Approx(3.0f).epsilon(0.05f));
}

TEST_CASE("Physics: derive a petit dt avec scene complete et Step")
{
    PhysicsWorld world;

    ShapeDesc floorShape;
    floorShape.kind = ShapeKind::Box;
    floorShape.halfExtent = glm::vec3(6.0f, 0.5f, 6.0f);
    RigidBody floor(world, floorShape, glm::vec3(0.0f, -1.5f, 0.0f),
                    glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Static);

    RigidBody cube(world, ShapeDesc{}, glm::vec3(0.0f, 0.5f, 0.0f));

    ShapeDesc sphereShape;
    sphereShape.kind = ShapeKind::Sphere;
    sphereShape.radius = 0.5f;
    RigidBody sphere(world, sphereShape, glm::vec3(-1.5f, 2.0f, 0.0f),
                     glm::quat(1.0f, 0.0f, 0.0f, 0.0f), MotionType::Dynamic, 0.5f, 0.35f);
    sphere.SetLinearVelocity(glm::vec3(2.2f, 0.0f, 0.0f));

    for (int i = 0; i < 6; ++i)
    {
        const float angle = static_cast<float>(i) * glm::two_pi<float>() / 6.0f;
        RigidBody orb(world, ShapeDesc{}, glm::vec3(std::cos(angle) * 2.2f, 2.0f + i * 0.5f,
                                                    std::sin(angle) * 2.2f));
        orb.SetLinearVelocity(glm::vec3(-std::sin(angle) * 1.5f, 0.0f, std::cos(angle) * 1.5f));
    }

    PlayerCharacter player(world, glm::vec3(0.0f, 0.2f, 3.0f));

    const float kSmallDt = 0.0009f;
    for (int i = 0; i < 600; ++i)
    {
        world.Step(kSmallDt);
        player.Update(kSmallDt, glm::vec3(0.0f), false);
    }

    const glm::vec3 p = player.GetPosition();
    CHECK(p.x == doctest::Approx(0.0f).epsilon(0.05f));
    CHECK(p.z == doctest::Approx(3.0f).epsilon(0.05f));
}
