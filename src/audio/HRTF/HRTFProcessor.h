#pragma once

namespace synapse {

class HRTFProcessor
{
public:
    HRTFProcessor() = default;
    ~HRTFProcessor() = default;

    HRTFProcessor(const HRTFProcessor&) = delete;
    HRTFProcessor& operator=(const HRTFProcessor&) = delete;
};

} // namespace synapse
