#pragma once

namespace synapse {

class CollisionSystem
{
public:
    CollisionSystem() = default;
    ~CollisionSystem() = default;

    CollisionSystem(const CollisionSystem&) = delete;
    CollisionSystem& operator=(const CollisionSystem&) = delete;
};

} // namespace synapse
