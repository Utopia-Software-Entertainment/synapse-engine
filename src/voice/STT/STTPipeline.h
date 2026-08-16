#pragma once

#include <voice/STT/STTBackend.h>

#include <memory>

namespace synapse {
namespace voice {

// ─────────────────────────────────────────────────────────────────────────
// STTPipeline — orchestrateur de la reconnaissance vocale du moteur.
// Choisit le backend (whisper.cpp par défaut, CrisperWhisper optionnel),
// en fonction de config/ai_toolchain.json. Consommé par le système
// d'incantation vocale (VoiceRecognizer / IntentParser / CommandMatcher).
//
// Cible de latence sur ce PC (Ryzen 7, CPU, modèle small) : ~200-400 ms
// pour une commande de 1-2 s, sans toucher la VRAM (rendu VR 90 FPS intact).
// ─────────────────────────────────────────────────────────────────────────

class STTPipeline
{
public:
    enum class Backend
    {
        Auto,          // → WhisperCpp
        WhisperCpp,    // production (poids MIT/Apache)
        CrisperWhisper // prototypage uniquement (poids NON-COMMERCIAL)
    };

    struct PipelineConfig
    {
        Backend              backend = Backend::Auto;
        STTConfig            stt;                    // modelPath + useGpu
        std::string          whisperModelPath;       // repli prod (whisper.cpp)
        std::string          crisperCliPath = "crisper-whisper";
        STTOptions           options;                // langue, verbatim, timestamps
    };

    // Charge config/ai_toolchain.json → sélectionne le backend :
    //   - backend marqué "primary": true (CrisperWhisper par défaut),
    //   - sinon repli whisper.cpp.
    static PipelineConfig LoadConfigFromToolchain(const std::string& toolchainPath);

    STTPipeline() = default;
    ~STTPipeline();

    STTPipeline(const STTPipeline&) = delete;
    STTPipeline& operator=(const STTPipeline&) = delete;

    bool Init(const PipelineConfig& config);
    void Shutdown();
    bool IsReady() const { return m_Backend != nullptr; }

    const char* BackendName() const;

    // PCM mono float, 16 kHz. Retourne texte + timestamps mot-à-mot.
    STTResult Transcribe(const f32* pcm, usize nSamples, u32 sampleRate);

    const STTOptions& GetOptions() const { return m_Options; }

private:
    std::unique_ptr<ISTTBackend> m_Backend;
    STTOptions m_Options;
};

} // namespace voice
} // namespace synapse