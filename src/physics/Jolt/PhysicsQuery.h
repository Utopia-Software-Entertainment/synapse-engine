#pragma once

namespace synapse {

class PhysicsQuery
{
public:
    PhysicsQuery() = default;
    ~PhysicsQuery() = default;

    PhysicsQuery(const PhysicsQuery&) = delete;
    PhysicsQuery& operator=(const PhysicsQuery&) = delete;
};

} // namespace synapse
