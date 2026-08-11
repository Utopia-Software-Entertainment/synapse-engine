#pragma once

namespace synapse {

class AudioSource
{
public:
    AudioSource() = default;
    ~AudioSource() = default;

    AudioSource(const AudioSource&) = delete;
    AudioSource& operator=(const AudioSource&) = delete;
};

} // namespace synapse
