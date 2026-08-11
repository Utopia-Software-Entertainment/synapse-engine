#pragma once

namespace synapse {

class AudioEngine
{
public:
    AudioEngine() = default;
    ~AudioEngine() = default;

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
};

} // namespace synapse
