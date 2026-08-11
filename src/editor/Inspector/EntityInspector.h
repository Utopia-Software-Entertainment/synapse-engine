#pragma once

namespace synapse {

class EntityInspector
{
public:
    EntityInspector() = default;
    ~EntityInspector() = default;

    EntityInspector(const EntityInspector&) = delete;
    EntityInspector& operator=(const EntityInspector&) = delete;
};

} // namespace synapse
