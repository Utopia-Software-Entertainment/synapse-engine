#pragma once

namespace synapse {

class BioInputBridge
{
public:
    BioInputBridge() = default;
    ~BioInputBridge() = default;

    BioInputBridge(const BioInputBridge&) = delete;
    BioInputBridge& operator=(const BioInputBridge&) = delete;
};

} // namespace synapse
