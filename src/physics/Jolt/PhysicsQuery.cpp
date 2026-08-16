#include <physics/Jolt/PhysicsQuery.h>
#include <physics/Jolt/PhysicsWorld.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseStats.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace synapse::physics {

std::optional<RaycastHit> PhysicsQuery::RayCast(PhysicsWorld& world, const glm::vec3& origin,
                                                 const glm::vec3& direction, float maxDistance)
{
    JPH::PhysicsSystem& system = world.GetPhysicsSystem();
    JPH::RRayCast ray{ JPH::RVec3(origin.x, origin.y, origin.z), JPH::Vec3(direction.x * maxDistance, direction.y * maxDistance, direction.z * maxDistance) };
    JPH::RayCastResult result;

    if (system.GetNarrowPhaseQuery().CastRay(ray, result))
    {
        RaycastHit hit;
        hit.distance = result.mFraction * maxDistance;
        hit.position = origin + direction * hit.distance;
        hit.bodyID = result.mBodyID.GetIndex();

        // Get normal
        JPH::BodyLockRead lock(system.GetBodyLockInterface(), result.mBodyID);
        if (lock.Succeeded())
        {
            const JPH::Body& body = lock.GetBody();
            JPH::Vec3 normal = body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, ray.GetPointOnRay(result.mFraction));
            hit.normal = glm::vec3(normal.GetX(), normal.GetY(), normal.GetZ());
        }

        return hit;
    }

    return std::nullopt;
}

} // namespace synapse::physics
