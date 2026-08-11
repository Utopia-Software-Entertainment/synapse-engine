#pragma once

namespace synapse {

class FFTProcessor
{
public:
    FFTProcessor() = default;
    ~FFTProcessor() = default;

    FFTProcessor(const FFTProcessor&) = delete;
    FFTProcessor& operator=(const FFTProcessor&) = delete;
};

} // namespace synapse
