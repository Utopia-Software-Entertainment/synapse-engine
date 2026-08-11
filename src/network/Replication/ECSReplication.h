#pragma once

namespace synapse {

class ECSReplication
{
public:
    ECSReplication() = default;
    ~ECSReplication() = default;

    ECSReplication(const ECSReplication&) = delete;
    ECSReplication& operator=(const ECSReplication&) = delete;
};

} // namespace synapse
