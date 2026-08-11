#pragma once

namespace synapse {

class NetworkSocket
{
public:
    NetworkSocket() = default;
    ~NetworkSocket() = default;

    NetworkSocket(const NetworkSocket&) = delete;
    NetworkSocket& operator=(const NetworkSocket&) = delete;
};

} // namespace synapse
