#include <voice/STT/WhisperBackend.h>

#include <core/Logger.h>

#include <whisper.h>

#include <algorithm>
#include <chrono>
#include <string>

namespace synapse {
namespace voice {

WhisperBackend::~WhisperBackend()
{
    Shutdown();
}

bool WhisperBackend::Init(const STTConfig& config)
{
    if (config.modelPath.empty())
    {
        SYNAPSE_CORE_ERROR("WhisperBackend: modelPath vide (voir scripts/tools/fetch_models.sh)");
        return false;
    }

    whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = config.useGpu; // CPU par défaut → VRAM libre pour le rendu VR
    cparams.flash_attn = false;

    m_Ctx = whisper_init_from_file_with_params(config.modelPath.c_str(), cparams);
    if (!m_Ctx)
    {
        SYNAPSE_CORE_ERROR("WhisperBackend: echec d'initialisation du modele '{}'", config.modelPath);
        return false;
    }

    m_Config = config;
    SYNAPSE_CORE_INFO("WhisperBackend pret (modele={}, gpu={})", config.modelPath, config.useGpu);
    return true;
}

void WhisperBackend::Shutdown()
{
    if (m_Ctx)
    {
        whisper_free(m_Ctx);
        m_Ctx = nullptr;
    }
}

STTResult WhisperBackend::Transcribe(const f32* pcm, usize nSamples, u32 sampleRate,
                                     const STTOptions& options)
{
    STTResult result;
    result.mode = options.mode;
    result.language = options.language;

    if (!m_Ctx || !pcm || nSamples == 0)
    {
        result.error = "backend non initialise ou buffer vide";
        return result;
    }

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_realtime  = false;
    params.print_progress  = false;
    params.print_timestamps = false;
    params.n_threads       = std::max(1u, options.threads);
    params.language        = options.language.c_str();
    params.no_timestamps   = false;
    params.token_timestamps = options.wordTimestamps;
    params.max_tokens      = static_cast<int>(options.maxTokens);
    params.offset_ms       = 0;
    params.duration_ms     = static_cast<int>(options.maxDurationMs); // fenêtre de commande vocale
    params.single_segment  = false;

    const auto t0 = std::chrono::steady_clock::now();
    const int r = whisper_full(m_Ctx, params, pcm, static_cast<int>(nSamples));
    const f64 ms = std::chrono::duration<f64, std::milli>(std::chrono::steady_clock::now() - t0).count();
    result.latencyMs = ms;

    if (r != 0)
    {
        result.error = "whisper_full a echoue (code " + std::to_string(r) + ")";
        return result;
    }

    const int nSeg = whisper_full_n_segments(m_Ctx);
    if (nSeg <= 0)
    {
        result.ok = true; // silence détecté, rien à transcrire
        return result;
    }

    std::string fullText;
    for (int s = 0; s < nSeg; ++s)
    {
        const char* segText = whisper_full_get_segment_text(m_Ctx, s);
        if (segText)
        {
            fullText += segText;
        }

        if (!options.wordTimestamps)
        {
            continue;
        }

        const int nTok = whisper_full_n_tokens(m_Ctx, s);
        for (int t = 0; t < nTok; ++t)
        {
            const whisper_token_data td = whisper_full_get_token_data(m_Ctx, s, t);
            const char* tok = whisper_token_to_str(m_Ctx, td.id);
            if (!tok || tok[0] == '\0' || tok[0] == '[')
            {
                continue; // skip tokens spéciaux ([BEG], [SOT], ...)
            }

            WordTimestamp w;
            w.startSeconds = static_cast<f64>(td.t0) / 1000.0;
            w.endSeconds   = static_cast<f64>(td.t1) / 1000.0;
            w.word         = tok;
            result.words.push_back(std::move(w));
        }
    }

    result.text = fullText;
    result.ok   = !fullText.empty();
    return result;
}

} // namespace voice
} // namespace synapse