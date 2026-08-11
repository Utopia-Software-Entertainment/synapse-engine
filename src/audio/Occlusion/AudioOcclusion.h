#pragma once

namespace synapse {

class AudioOcclusion
{
public:
    AudioOcclusion() = default;
    ~AudioOcclusion() = default;

    AudioOcclusion(const AudioOcclusion&) = delete;
    AudioOcclusion& operator=(const AudioOcclusion&) = delete;
};

} // namespace synapse
