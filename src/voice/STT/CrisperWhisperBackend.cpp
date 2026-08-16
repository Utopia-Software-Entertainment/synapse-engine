#include <voice/STT/CrisperWhisperBackend.h>

#include <core/Logger.h>

#include <nlohmann/json.hpp>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <thread>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace synapse {
namespace voice {

CrisperWhisperBackend::~CrisperWhisperBackend()
{
    Shutdown();
}

bool CrisperWhisperBackend::Init(const STTConfig& config)
{
    m_Config = static_cast<const CrisperWhisperConfig&>(config);

    // Détection du CLI au premier init. Le téléchargement est documenté dans
    // config/ai_toolchain.json (repo Saganaki22/CrisperWhisper.cpp).
    std::string probe = m_Config.cliPath + " --help";
    m_Available = (std::system(probe.c_str()) == 0);
    if (!m_Available)
    {
        SYNAPSE_CORE_WARN("CrisperWhisperBackend: CLI '{}' introuvable. "
                          "Backend desactive (prototypage seulement, poids non-commerciaux).",
                          m_Config.cliPath);
        return false;
    }

    SYNAPSE_CORE_INFO("CrisperWhisperBackend pret (cli={}, modele={})", m_Config.cliPath,
                      m_Config.modelPath);
    return true;
}

void CrisperWhisperBackend::Shutdown()
{
    m_Available = false;
}

STTResult CrisperWhisperBackend::Transcribe(const f32* pcm, usize nSamples, u32 sampleRate,
                                            const STTOptions& options)
{
    STTResult result;
    result.mode = options.mode;
    result.language = options.language;

    if (!m_Available)
    {
        result.error = "backend CrisperWhisper indisponible (CLI introuvable)";
        return result;
    }
    if (!pcm || nSamples == 0)
    {
        result.error = "buffer vide";
        return result;
    }

    const fs::path wavPath = fs::temp_directory_path() /
                             ("synapse_crisper_" + std::to_string(
                                 std::chrono::steady_clock::now().time_since_epoch().count()) + ".wav");
    if (!WriteWav(pcm, nSamples, sampleRate, wavPath.string().c_str()))
    {
        result.error = "echec ecriture WAV temporaire";
        return result;
    }

    const char* mode = (options.mode == STTMode::Verbatim) ? "verbatim" : "intended";
    std::string cmd = m_Config.cliPath + " -m \"" + m_Config.modelPath + "\" -f \"" +
                      wavPath.string() + "\" --mode " + mode + " --word-timestamps --json -l " +
                      options.language + " 2>/dev/null";

    const auto t0 = std::chrono::steady_clock::now();

    // Capture stdout via popen pour parser le JSON.
    std::string out;
    if (FILE* pipe = popen(cmd.c_str(), "r"))
    {
        std::array<char, 4096> buf{};
        while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr)
        {
            out += buf.data();
        }
        pclose(pipe);
    }

    const f64 ms = std::chrono::duration<f64, std::milli>(std::chrono::steady_clock::now() - t0).count();
    result.latencyMs = ms;
    fs::remove(wavPath);

    if (out.empty())
    {
        result.error = "CLI CrisperWhisper n'a rien retourne";
        return result;
    }

    try
    {
        const json doc = json::parse(out);
        if (doc.contains("text"))
        {
            result.text = doc["text"].get<std::string>();
        }
        if (doc.contains("words") && doc["words"].is_array())
        {
            for (const auto& w : doc["words"])
            {
                WordTimestamp wt;
                if (w.contains("word")) wt.word = w["word"].get<std::string>();
                if (w.contains("start")) wt.startSeconds = w["start"].get<f64>();
                if (w.contains("end"))   wt.endSeconds = w["end"].get<f64>();
                result.words.push_back(std::move(wt));
            }
        }
        result.ok = !result.text.empty();
    }
    catch (const json::exception& e)
    {
        // JSON mal formé : on garde la sortie brute comme texte de secours.
        result.text = out;
        result.ok = true;
        SYNAPSE_CORE_WARN("CrisperWhisperBackend: JSON illisible ({}) — texte brut conserve", e.what());
    }

    return result;
}

bool CrisperWhisperBackend::WriteWav(const f32* pcm, usize nSamples, u32 sampleRate,
                                     const char* path) const
{
    FILE* f = std::fopen(path, "wb");
    if (!f)
    {
        return false;
    }

    const u32 numChannels = 1;
    const u32 bitsPerSample = 16;
    const u32 byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    const u32 blockAlign = numChannels * (bitsPerSample / 8);
    const u32 dataSize = static_cast<u32>(nSamples) * (bitsPerSample / 8);

    const auto writeStr = [f](const char* s) { std::fwrite(s, 1, 4, f); };
    const auto writeU32 = [f](u32 v) { std::fwrite(&v, sizeof(u32), 1, f); };
    const auto writeU16 = [f](u16 v) { std::fwrite(&v, sizeof(u16), 1, f); };

    writeStr("RIFF");
    writeU32(36 + dataSize);
    writeStr("WAVE");
    writeStr("fmt ");
    writeU32(16);
    writeU16(1); // PCM
    writeU16(static_cast<u16>(numChannels));
    writeU32(sampleRate);
    writeU32(byteRate);
    writeU16(static_cast<u16>(blockAlign));
    writeU16(static_cast<u16>(bitsPerSample));
    writeStr("data");
    writeU32(dataSize);

    // Conversion f32 [-1,1] → s16 little-endian, clampé.
    for (usize i = 0; i < nSamples; ++i)
    {
        f32 v = pcm[i];
        v = v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v);
        const i16 s = static_cast<i16>(v * 32767.0f);
        std::fwrite(&s, sizeof(i16), 1, f);
    }

    std::fclose(f);
    return true;
}

} // namespace voice
} // namespace synapse