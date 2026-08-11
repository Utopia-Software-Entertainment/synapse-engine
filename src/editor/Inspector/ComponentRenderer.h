#pragma once

namespace synapse {

class ComponentRenderer
{
public:
    ComponentRenderer() = default;
    ~ComponentRenderer() = default;

    ComponentRenderer(const ComponentRenderer&) = delete;
    ComponentRenderer& operator=(const ComponentRenderer&) = delete;
};

} // namespace synapse
