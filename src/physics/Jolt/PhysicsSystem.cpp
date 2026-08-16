#include <physics/Jolt/PhysicsSystem.h>
#include <physics/Jolt/PhysicsWorld.h>
#include <core/ECS/Components/TransformComponent.h>
#include <core/ECS/Components/PhysicsBodyComponent.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>

namespace synapse::physics {

void PhysicsSystem::Update(f32 deltaTime, PhysicsWorld& world, synapse::ecs::Registry& registry)
{
    // Step the physics world
    world.Step(deltaTime);

    auto& bodyInterface = world.GetBodyInterface();

    // Synchronize Physics -> ECS
    auto view = registry.view<synapse::ecs::TransformComponent, synapse::ecs::PhysicsBodyComponent>();
    for (auto entity : view)
    {
        auto& transform = view.get<synapse::ecs::TransformComponent>(entity);
        const auto& physics = view.get<synapse::ecs::PhysicsBodyComponent>(entity);

        if (physics.type == ecs::BodyType::Static) continue;

        JPH::RVec3 pos;
        JPH::Quat rot;
        JPH::BodyID id(physics.bodyIDValue);
        bodyInterface.GetPositionAndRotation(id, pos, rot);

        transform.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ()) - physics.offset;
        transform.rotation = glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
    }
}

} // namespace synapse::physics
