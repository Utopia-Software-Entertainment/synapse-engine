#pragma once

namespace synapse {

class ShadowPass
{
public:
    ShadowPass() = default;
    ~ShadowPass() = default;

    ShadowPass(const ShadowPass&) = delete;
    ShadowPass& operator=(const ShadowPass&) = delete;
};

} // namespace synapse
