#pragma once

#include <core/Types.h>
#include <core/ECS/Registry.h>

namespace synapse::physics {

class PhysicsWorld;

class PhysicsSystem
{
public:
    // Synchronize ECS transforms with physics bodies and step the world
    static void Update(f32 deltaTime, PhysicsWorld& world, synapse::ecs::Registry& registry);
};

} // namespace synapse::physics
