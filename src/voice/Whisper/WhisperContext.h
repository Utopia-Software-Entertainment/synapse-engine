#pragma once

namespace synapse {

class WhisperContext
{
public:
    WhisperContext() = default;
    ~WhisperContext() = default;

    WhisperContext(const WhisperContext&) = delete;
    WhisperContext& operator=(const WhisperContext&) = delete;
};

} // namespace synapse
