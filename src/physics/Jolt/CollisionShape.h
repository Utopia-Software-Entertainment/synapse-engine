#pragma once

namespace synapse {

class CollisionShape
{
public:
    CollisionShape() = default;
    ~CollisionShape() = default;

    CollisionShape(const CollisionShape&) = delete;
    CollisionShape& operator=(const CollisionShape&) = delete;
};

} // namespace synapse
