#pragma once

namespace synapse {

class FoveatedPass
{
public:
    FoveatedPass() = default;
    ~FoveatedPass() = default;

    FoveatedPass(const FoveatedPass&) = delete;
    FoveatedPass& operator=(const FoveatedPass&) = delete;
};

} // namespace synapse
