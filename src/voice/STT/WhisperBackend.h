#pragma once

#include <voice/STT/STTBackend.h>

#include <string>

struct whisper_context;

namespace synapse {
namespace voice {

// Backend production : whisper.cpp (ggerganov, MIT).
// Poids OpenAI Whisper (MIT/Apache-2.0) → utilisable commercialement.
// Fournit des timestamps token-level via whisper_full_get_token_data.
class WhisperBackend final : public ISTTBackend
{
public:
    WhisperBackend() = default;
    ~WhisperBackend() override;

    bool Init(const STTConfig& config) override;
    void Shutdown() override;
    const char* Name() const override { return "whisper.cpp"; }

    STTResult Transcribe(const f32* pcm, usize nSamples, u32 sampleRate,
                         const STTOptions& options) override;

private:
    struct whisper_context* m_Ctx = nullptr;
    STTConfig m_Config;
};

} // namespace voice
} // namespace synapse