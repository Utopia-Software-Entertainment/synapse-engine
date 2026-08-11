#pragma once

namespace synapse {

class AudioSystem
{
public:
    AudioSystem() = default;
    ~AudioSystem() = default;

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
};

} // namespace synapse
