#pragma once

#include <core/Types.h>

#include <string>
#include <vector>

namespace synapse {
namespace voice {

// ─────────────────────────────────────────────────────────────────────────
// Abstraction STT (Speech-To-Text). Deux backends :
//   - WhisperBackend       : whisper.cpp (production, poids MIT/Apache)
//   - CrisperWhisperBackend: bridge CLI CrisperWhisper 2 (prototypage,
//                            poids NON-COMMERCIAL — voir config/ai_toolchain.json)
//
// Critère clé du moteur (incantations vocales "Boule de feu") :
// timestamps mot-à-mot précis + mode verbatim (cris, murmures, hésitations).
// ─────────────────────────────────────────────────────────────────────────

struct WordTimestamp
{
    f64         startSeconds = 0.0;
    f64         endSeconds   = 0.0;
    std::string word;
};

enum class STTMode
{
    Verbatim, // ce qui est réellement dit (fillers, répétitions, hésitations)
    Intended  // texte propre, ce que le locuteur voulait dire
};

struct STTOptions
{
    std::string language = "fr";
    STTMode     mode     = STTMode::Verbatim;
    bool        wordTimestamps = true;
    u32         maxTokens      = 256;
    u32         threads        = 4;
    f32         maxDurationMs  = 3000.0f; // fenêtre de commande vocale (sort)
};

struct STTResult
{
    bool        ok         = false;
    std::string error;
    std::string text;
    std::string language;
    STTMode     mode       = STTMode::Verbatim;
    f64         latencyMs  = 0.0;
    std::vector<WordTimestamp> words;
};

struct STTConfig
{
    std::string modelPath;  // ex: assets/models/whisper/ggml-small.bin
    bool        useGpu = false; // CPU par défaut : préserve la VRAM pour le rendu VR
};

class ISTTBackend
{
public:
    virtual ~ISTTBackend() = default;

    virtual bool Init(const STTConfig& config) = 0;
    virtual void Shutdown() = 0;
    virtual const char* Name() const = 0;

    // PCM mono float, sampleRate 16 kHz attendu
    virtual STTResult Transcribe(const f32* pcm, usize nSamples, u32 sampleRate,
                                 const STTOptions& options) = 0;
};

} // namespace voice
} // namespace synapse