#pragma once

namespace synapse {

class RigidBody
{
public:
    RigidBody() = default;
    ~RigidBody() = default;

    RigidBody(const RigidBody&) = delete;
    RigidBody& operator=(const RigidBody&) = delete;
};

} // namespace synapse
