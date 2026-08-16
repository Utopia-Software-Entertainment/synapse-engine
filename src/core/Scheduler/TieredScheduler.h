#pragma once

#include <core/Types.h>
#include <taskflow/taskflow.hpp>

#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

namespace synapse {

// ─────────────────────────────────────────────────────────────────────────
// TieredScheduler — découpe hiérarchique 3 niveaux inspirée de
// Gemini Robotics 2 (vidéo Vision IA, [12:05]/[12:52]).
//
//   1. RÉFLEXE  : thread principal, haute fréquence (défaut 90 Hz).
//      Jamais bloquant : eye-tracking, EMG, input, gaze raycast.
//   2. ACTION   : thread principal, fréquence moyenne (défaut 60 Hz).
//      Physique, animation.
//   3. CERVEAU  : pool de threads asynchrone (taskflow), fréquence libre.
//      IA PNJ, dialogues, pathfinding planning — déportés pour ne jamais
//      bloquer la boucle de rendu VR (anti motion-sickness).
// ─────────────────────────────────────────────────────────────────────────

using TierFn = std::function<void(f32 deltaTime)>;
using BrainFn = std::function<void()>;

struct BrainJobDesc
{
    std::string name;
    BrainFn     fn;
};

class TieredScheduler
{
public:
    TieredScheduler() = default;
    ~TieredScheduler();

    TieredScheduler(const TieredScheduler&) = delete;
    TieredScheduler& operator=(const TieredScheduler&) = delete;

    // ── Réflexe : thread principal, haute fréquence ──
    void AddReflexSystem(TierFn fn, const char* name = "reflex_system");

    // ── Action : thread principal, fréquence moyenne ──
    void AddActionSystem(TierFn fn, const char* name = "action_system");

    // ── Cerveau : pool taskflow asynchrone ──
    void AddBrainTask(BrainJobDesc&& desc);

    // ── Config (voir config/ai_toolchain.json → tiered_ecs_gemini) ──
    void SetReflexHz(f32 hz) { m_ReflexHz = hz; }
    void SetActionHz(f32 hz) { m_ActionHz = hz; }
    void SetBrainThreads(u32 threads);

    void Start();
    void Shutdown();

    // À appeler une fois par frame sur le thread principal (avant le render)
    void Tick(f32 frameDeltaTime);

    struct Stats
    {
        f32 reflexDtUs   = 0.0f; // coût moyen du niveau réflexe (µs)
        f32 actionDtUs   = 0.0f; // coût moyen du niveau action (µs)
        u32 brainRunning = 0;    // jobs cerveau en vol
    };
    const Stats& GetStats() const { return m_Stats; }

private:
    void RunSystems(f32 dt, std::vector<TierFn>& systems, const char* tier);
    void PollBrain();

    std::vector<TierFn> m_ReflexSystems;
    std::vector<TierFn> m_ActionSystems;

    struct BrainJob
    {
        std::string          name;
        BrainFn              fn;
        std::future<void>    future;
    };
    std::vector<BrainJob> m_BrainJobs;

    std::unique_ptr<tf::Executor> m_BrainExecutor;
    u32                           m_BrainThreads = 6;

    f32 m_ReflexHz = 90.0f;
    f32 m_ActionHz = 60.0f;

    std::chrono::steady_clock::time_point m_LastReflex;
    std::chrono::steady_clock::time_point m_LastAction;
    bool m_Running = false;

    Stats m_Stats;
};

} // namespace synapse