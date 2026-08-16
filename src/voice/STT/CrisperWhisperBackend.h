#pragma once

#include <voice/STT/STTBackend.h>

namespace synapse {
namespace voice {

// Backend optionnel (prototypage uniquement) : pont CLI vers
// CrisperWhisper.cpp (https://github.com/Saganaki22/CrisperWhisper.cpp).
//
// ⚠ LICENCE : le CODE CrisperWhisper.cpp est MIT, mais les POIDS
// CrisperWhisper 2.0 (nyralabs) sont sous Non-Commercial Research License.
// → NE PAS utiliser dans le jeu commercial. Réserver au prototypage.
//
// Le CLI est invoqué en sous-processus sur un fichier WAV temporaire :
//   crisper-whisper -m <model.bin> -f <wav> --mode verbatim|intended \
//       --word-timestamps --json -l <lang>
struct CrisperWhisperConfig : STTConfig
{
    std::string cliPath = "crisper-whisper";
};

class CrisperWhisperBackend final : public ISTTBackend
{
public:
    CrisperWhisperBackend() = default;
    ~CrisperWhisperBackend() override;

    bool Init(const STTConfig& config) override;
    void Shutdown() override;
    const char* Name() const override { return "crisper-whisper.cpp"; }

    STTResult Transcribe(const f32* pcm, usize nSamples, u32 sampleRate,
                         const STTOptions& options) override;

private:
    bool WriteWav(const f32* pcm, usize nSamples, u32 sampleRate, const char* path) const;

    CrisperWhisperConfig m_Config;
    bool m_Available = false;
};

} // namespace voice
} // namespace synapse