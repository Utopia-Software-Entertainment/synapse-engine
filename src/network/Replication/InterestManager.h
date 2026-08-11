#pragma once

namespace synapse {

class InterestManager
{
public:
    InterestManager() = default;
    ~InterestManager() = default;

    InterestManager(const InterestManager&) = delete;
    InterestManager& operator=(const InterestManager&) = delete;
};

} // namespace synapse
