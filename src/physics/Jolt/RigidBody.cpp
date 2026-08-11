#include <physics/Jolt/RigidBody.h>

#include <physics/Jolt/PhysicsWorld.h>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include <glm/gtc/matrix_transform.hpp>

namespace synapse::physics {

RigidBody::RigidBody(PhysicsWorld& world, const ShapeDesc& shape, const glm::vec3& position,
                     const glm::quat& rotation, MotionType motion, f32 friction,
                     f32 restitution)
    : m_World(world), m_BodyID(nullptr)
{
    switch (shape.kind)
    {
    case ShapeKind::Box:
        m_Shape = new JPH::BoxShape(
            JPH::Vec3(shape.halfExtent.x, shape.halfExtent.y, shape.halfExtent.z));
        break;
    case ShapeKind::Sphere:
        m_Shape = new JPH::SphereShape(shape.radius);
        break;
    case ShapeKind::Capsule:
        m_Shape = new JPH::CapsuleShape(shape.halfHeight, shape.radius);
        break;
    }

    JPH::BodyCreationSettings settings(
        m_Shape, JPH::RVec3(position.x, position.y, position.z),
        JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w),
        motion == MotionType::Static ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
        static_cast<JPH::ObjectLayer>(motion == MotionType::Static ? ObjectLayer::Static
                                                                   : ObjectLayer::Dynamic));
    settings.mFriction = friction;
    settings.mRestitution = restitution;

    const JPH::BodyID id = m_World.GetBodyInterface().CreateAndAddBody(
        settings, JPH::EActivation::Activate);
    m_BodyID = new JPH::BodyID(id);
}

RigidBody::~RigidBody()
{
    if (m_BodyID != nullptr)
    {
        auto& interface = m_World.GetBodyInterface();
        interface.RemoveBody(*m_BodyID);
        interface.DestroyBody(*m_BodyID);
        delete m_BodyID;
    }
}

glm::mat4 RigidBody::GetModelMatrix() const
{
    const JPH::Vec3 pos = m_World.GetBodyInterface().GetCenterOfMassPosition(*m_BodyID);
    const JPH::Quat rot = m_World.GetBodyInterface().GetRotation(*m_BodyID);
    const glm::mat4 translate = glm::translate(glm::mat4(1.0f),
                                               glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()));
    const glm::mat4 rotate = glm::mat4_cast(glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()));
    return translate * rotate;
}

glm::vec3 RigidBody::GetPosition() const
{
    const JPH::Vec3 pos = m_World.GetBodyInterface().GetCenterOfMassPosition(*m_BodyID);
    return glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
}

glm::quat RigidBody::GetRotation() const
{
    const JPH::Quat rot = m_World.GetBodyInterface().GetRotation(*m_BodyID);
    return glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
}

void RigidBody::SetLinearVelocity(const glm::vec3& velocity)
{
    m_World.GetBodyInterface().SetLinearVelocity(*m_BodyID,
                                                 JPH::Vec3(velocity.x, velocity.y, velocity.z));
}

} // namespace synapse::physics
