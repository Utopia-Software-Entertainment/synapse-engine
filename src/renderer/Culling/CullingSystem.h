#pragma once

namespace synapse {

class CullingSystem
{
public:
    CullingSystem() = default;
    ~CullingSystem() = default;

    CullingSystem(const CullingSystem&) = delete;
    CullingSystem& operator=(const CullingSystem&) = delete;
};

} // namespace synapse
