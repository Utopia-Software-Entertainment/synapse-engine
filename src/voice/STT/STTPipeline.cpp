#include <voice/STT/STTPipeline.h>

#include <voice/STT/CrisperWhisperBackend.h>
#include <voice/STT/WhisperBackend.h>

#include <core/Logger.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

using json = nlohmann::json;

namespace synapse {
namespace voice {

STTPipeline::~STTPipeline()
{
    Shutdown();
}

STTPipeline::PipelineConfig STTPipeline::LoadConfigFromToolchain(const std::string& toolchainPath)
{
    PipelineConfig cfg;
    cfg.options.language      = "fr";
    cfg.options.mode          = STTMode::Verbatim;
    cfg.options.wordTimestamps = true;

    try
    {
        std::ifstream f(toolchainPath);
        if (!f)
        {
            SYNAPSE_CORE_WARN("STTPipeline: config introuvable '{}' — défauts whisper.cpp", toolchainPath);
            return cfg;
        }
        const json doc = json::parse(f);

        for (const auto& tool : doc.value("tools", json::array()))
        {
            if (tool.value("id", "") != "crisper_whisper2")
            {
                continue;
            }
            cfg.options.language = tool.value("language", "fr");

            if (!tool.contains("backends"))
            {
                break;
            }
            const auto& bc = tool["backends"];

            // Repli production (poids MIT) toujours connu.
            if (bc.contains("whisper_cpp") && bc["whisper_cpp"].contains("model_path"))
            {
                cfg.whisperModelPath = bc["whisper_cpp"]["model_path"];
            }

            // Backend PRINCIPAL = celui marqué "primary": true dans la config.
            // (CrisperWhisper par défaut ; repli whisper.cpp si CLI absent.)
            if (bc.contains("crisperwhisper_cpp") &&
                bc["crisperwhisper_cpp"].value("primary", false))
            {
                const auto& cw = bc["crisperwhisper_cpp"];
                cfg.backend       = Backend::CrisperWhisper;
                cfg.crisperCliPath = cw.value("cli_path", "crisper-whisper");
                if (cw.contains("model") && cw["model"].contains("path"))
                {
                    cfg.stt.modelPath = cw["model"]["path"];
                }
                SYNAPSE_CORE_INFO("STTPipeline: backend principal CrisperWhisper sélectionné (config)");
            }
            else
            {
                cfg.backend = Backend::WhisperCpp;
                cfg.stt.modelPath = cfg.whisperModelPath;
            }
            break;
        }
    }
    catch (const json::exception& e)
    {
        SYNAPSE_CORE_WARN("STTPipeline: config illisible ({}) — défauts whisper.cpp", e.what());
    }

    return cfg;
}

bool STTPipeline::Init(const PipelineConfig& config)
{
    m_Options = config.options;

    Backend chosen = config.backend;
    if (chosen == Backend::Auto)
    {
        chosen = Backend::WhisperCpp;
    }

    switch (chosen)
    {
        case Backend::CrisperWhisper:
        {
            auto backend = std::make_unique<CrisperWhisperBackend>();
            CrisperWhisperConfig cfg;
            cfg.modelPath = config.stt.modelPath;
            cfg.useGpu = config.stt.useGpu;
            cfg.cliPath = config.crisperCliPath;
            if (backend->Init(cfg))
            {
                m_Backend = std::move(backend);
            }
            else
            {
                SYNAPSE_CORE_WARN("STTPipeline: bascule sur whisper.cpp (CrisperWhisper indisponible)");
                m_Backend = std::make_unique<WhisperBackend>();
            }
            break;
        }
        case Backend::WhisperCpp:
        default:
        {
            auto backend = std::make_unique<WhisperBackend>();
            STTConfig sttCfg = config.stt;
            if (sttCfg.modelPath.empty())
            {
                sttCfg.modelPath = config.whisperModelPath;
            }
            if (backend->Init(sttCfg))
            {
                m_Backend = std::move(backend);
            }
            break;
        }
    }

    if (!m_Backend)
    {
        SYNAPSE_CORE_ERROR("STTPipeline: aucun backend STT initialise");
        return false;
    }

    SYNAPSE_CORE_INFO("STTPipeline pret (backend={}, langue={}, verbatim={})", BackendName(),
                      m_Options.language,
                      m_Options.mode == STTMode::Verbatim ? "oui" : "non");
    return true;
}

void STTPipeline::Shutdown()
{
    if (m_Backend)
    {
        m_Backend->Shutdown();
        m_Backend.reset();
    }
}

const char* STTPipeline::BackendName() const
{
    return m_Backend ? m_Backend->Name() : "none";
}

STTResult STTPipeline::Transcribe(const f32* pcm, usize nSamples, u32 sampleRate)
{
    if (!m_Backend)
    {
        STTResult result;
        result.error = "STTPipeline non initialise";
        return result;
    }
    return m_Backend->Transcribe(pcm, nSamples, sampleRate, m_Options);
}

} // namespace voice
} // namespace synapse