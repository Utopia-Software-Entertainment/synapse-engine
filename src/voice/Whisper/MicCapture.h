#pragma once

namespace synapse {

class MicCapture
{
public:
    MicCapture() = default;
    ~MicCapture() = default;

    MicCapture(const MicCapture&) = delete;
    MicCapture& operator=(const MicCapture&) = delete;
};

} // namespace synapse
