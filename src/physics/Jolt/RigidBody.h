#pragma once

#include <core/Types.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace JPH {

class BodyID;

} // namespace JPH

namespace synapse::physics {

class PhysicsWorld;

enum class ShapeKind
{
    Box,
    Sphere,
    Capsule,
};

enum class MotionType
{
    Static,
    Dynamic,
};

struct ShapeDesc
{
    ShapeKind kind = ShapeKind::Box;
    glm::vec3 halfExtent = glm::vec3(0.5f);
    f32 radius = 0.5f;
    f32 halfHeight = 0.5f;
};

class RigidBody
{
public:
    RigidBody(PhysicsWorld& world, const ShapeDesc& shape, const glm::vec3& position,
              const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
              MotionType motion = MotionType::Dynamic, f32 friction = 0.5f,
              f32 restitution = 0.2f);
    ~RigidBody();

    RigidBody(const RigidBody&) = delete;
    RigidBody& operator=(const RigidBody&) = delete;

    glm::mat4 GetModelMatrix() const;
    glm::vec3 GetPosition() const;
    glm::quat GetRotation() const;

    void SetLinearVelocity(const glm::vec3& velocity);

private:
    PhysicsWorld& m_World;
    JPH::Ref<JPH::Shape> m_Shape;
    JPH::BodyID* m_BodyID;
};

} // namespace synapse::physics
