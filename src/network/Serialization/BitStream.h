#pragma once

namespace synapse {

class BitStream
{
public:
    BitStream() = default;
    ~BitStream() = default;

    BitStream(const BitStream&) = delete;
    BitStream& operator=(const BitStream&) = delete;
};

} // namespace synapse
