#pragma once

namespace synapse {

class LSLReceiver
{
public:
    LSLReceiver() = default;
    ~LSLReceiver() = default;

    LSLReceiver(const LSLReceiver&) = delete;
    LSLReceiver& operator=(const LSLReceiver&) = delete;
};

} // namespace synapse
