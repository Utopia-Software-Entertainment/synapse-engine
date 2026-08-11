#pragma once

namespace synapse {

class BlinkDetector
{
public:
    BlinkDetector() = default;
    ~BlinkDetector() = default;

    BlinkDetector(const BlinkDetector&) = delete;
    BlinkDetector& operator=(const BlinkDetector&) = delete;
};

} // namespace synapse
