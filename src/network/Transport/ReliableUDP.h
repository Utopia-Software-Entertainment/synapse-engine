#pragma once

namespace synapse {

class ReliableUDP
{
public:
    ReliableUDP() = default;
    ~ReliableUDP() = default;

    ReliableUDP(const ReliableUDP&) = delete;
    ReliableUDP& operator=(const ReliableUDP&) = delete;
};

} // namespace synapse
