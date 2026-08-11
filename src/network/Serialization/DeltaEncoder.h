#pragma once

namespace synapse {

class DeltaEncoder
{
public:
    DeltaEncoder() = default;
    ~DeltaEncoder() = default;

    DeltaEncoder(const DeltaEncoder&) = delete;
    DeltaEncoder& operator=(const DeltaEncoder&) = delete;
};

} // namespace synapse
