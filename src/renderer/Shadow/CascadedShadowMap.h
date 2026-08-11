#pragma once

namespace synapse {

class CascadedShadowMap
{
public:
    CascadedShadowMap() = default;
    ~CascadedShadowMap() = default;

    CascadedShadowMap(const CascadedShadowMap&) = delete;
    CascadedShadowMap& operator=(const CascadedShadowMap&) = delete;
};

} // namespace synapse
