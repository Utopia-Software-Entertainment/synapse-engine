#pragma once

namespace synapse {

class PhysicsWorld
{
public:
    PhysicsWorld() = default;
    ~PhysicsWorld() = default;

    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;
};

} // namespace synapse
