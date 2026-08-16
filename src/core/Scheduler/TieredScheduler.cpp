#include <core/Scheduler/TieredScheduler.h>

#include <core/Logger.h>

#include <algorithm>
#include <thread>

namespace synapse {

TieredScheduler::~TieredScheduler()
{
    Shutdown();
}

void TieredScheduler::AddReflexSystem(TierFn fn, const char* name)
{
    (void)name;
    m_ReflexSystems.push_back(std::move(fn));
}

void TieredScheduler::AddActionSystem(TierFn fn, const char* name)
{
    (void)name;
    m_ActionSystems.push_back(std::move(fn));
}

void TieredScheduler::AddBrainTask(BrainJobDesc&& desc)
{
    m_BrainJobs.push_back({std::move(desc.name), std::move(desc.fn), {}});
}

void TieredScheduler::SetBrainThreads(u32 threads)
{
    m_BrainThreads = std::max(1u, threads);
}

void TieredScheduler::Start()
{
    if (m_Running)
    {
        return;
    }

    m_Running = true;
    m_LastReflex = std::chrono::steady_clock::now();
    m_LastAction = m_LastReflex;

    // Le pool "cerveau" consomme un sous-ensemble des threads CPU (16 sur ce
    // portable) pour ne pas étouffer la boucle de rendu. Default 6.
    m_BrainExecutor = std::make_unique<tf::Executor>(m_BrainThreads);

    SYNAPSE_CORE_INFO("TieredScheduler demarre: reflex={:.0f}Hz action={:.0f}Hz brain={} threads",
                      m_ReflexHz, m_ActionHz, m_BrainThreads);
}

void TieredScheduler::Shutdown()
{
    if (!m_Running)
    {
        return;
    }

    m_Running = false;
    if (m_BrainExecutor)
    {
        m_BrainExecutor->wait_for_all();
    }

    for (auto& job : m_BrainJobs)
    {
        if (job.future.valid())
        {
            job.future.wait();
        }
    }
}

void TieredScheduler::Tick(f32 frameDeltaTime)
{
    if (!m_Running)
    {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    const f32 reflexInterval = 1.0f / std::max(0.01f, m_ReflexHz);
    const f32 actionInterval = 1.0f / std::max(0.01f, m_ActionHz);

    // Réflexe : cadencé à m_ReflexHz, mais rattrape toujours au moins une
    // fois par frame si l'écart dépasse un intervalle (frame drop safe).
    const f32 reflexElapsed = std::chrono::duration<f32>(now - m_LastReflex).count();
    if (reflexElapsed >= reflexInterval)
    {
        const f32 dt = std::min(reflexElapsed, reflexInterval * 4.0f);
        RunSystems(dt, m_ReflexSystems, "reflex");
        m_LastReflex = now;
    }

    const f32 actionElapsed = std::chrono::duration<f32>(now - m_LastAction).count();
    if (actionElapsed >= actionInterval)
    {
        const f32 dt = std::min(actionElapsed, actionInterval * 4.0f);
        RunSystems(dt, m_ActionSystems, "action");
        m_LastAction = now;
    }

    (void)frameDeltaTime;
    PollBrain();
}

void TieredScheduler::RunSystems(f32 dt, std::vector<TierFn>& systems, const char* tier)
{
    const auto start = std::chrono::steady_clock::now();

    for (auto& system : systems)
    {
        if (system)
        {
            system(dt);
        }
    }

    const f32 elapsedUs = std::chrono::duration<f32, std::micro>(std::chrono::steady_clock::now() - start).count();
    if (std::string(tier) == "reflex")
    {
        m_Stats.reflexDtUs = elapsedUs;
    }
    else
    {
        m_Stats.actionDtUs = elapsedUs;
    }
}

void TieredScheduler::PollBrain()
{
    u32 running = 0;

    for (auto& job : m_BrainJobs)
    {
        // Un seul job en vol par système : on ne re-soumet que quand le
        // précédent est terminé, pour ne pas inonder le pool.
        const bool free = !job.future.valid() ||
                          job.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        if (free)
        {
            job.future = m_BrainExecutor->async([fn = job.fn]() { fn(); });
        }

        if (job.future.valid())
        {
            ++running;
        }
    }

    m_Stats.brainRunning = running;
}

} // namespace synapse