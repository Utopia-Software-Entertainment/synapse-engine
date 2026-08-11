#pragma once

namespace synapse {

class LightCullingPass
{
public:
    LightCullingPass() = default;
    ~LightCullingPass() = default;

    LightCullingPass(const LightCullingPass&) = delete;
    LightCullingPass& operator=(const LightCullingPass&) = delete;
};

} // namespace synapse
