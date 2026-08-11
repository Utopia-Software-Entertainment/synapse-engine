#pragma once

namespace synapse {

class VoiceRecognizer
{
public:
    VoiceRecognizer() = default;
    ~VoiceRecognizer() = default;

    VoiceRecognizer(const VoiceRecognizer&) = delete;
    VoiceRecognizer& operator=(const VoiceRecognizer&) = delete;
};

} // namespace synapse
