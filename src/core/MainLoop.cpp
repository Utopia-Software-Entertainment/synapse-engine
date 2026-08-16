#include <core/MainLoop.h>

#include <core/Logger.h>

#include <algorithm>
#include <chrono>

namespace synapse {

MainLoop::~MainLoop()
{
    RequestStop();
}

void MainLoop::SetSimulationFn(std::function<void(f32 fixedDt)> fn)
{
    m_SimulationFn = std::move(fn);
}

void MainLoop::SetRenderFn(std::function<void(const FrameState& frame)> fn)
{
    m_RenderFn = std::move(fn);
}

void MainLoop::Run()
{
    if (m_Running.exchange(true))
    {
        SYNAPSE_CORE_WARN("MainLoop::Run called while already running");
        return;
    }

    m_Workers.Start(4);

    m_RenderThread = std::thread([this]() { RenderThread(); });
    m_IoThread = std::thread([this]() { IoThread(); });

    Timer timer;
    timer.Reset();

    f32 accumulator = 0.0f;
    f32 totalTime = 0.0f;
    u64 frame = 0;

    SYNAPSE_CORE_INFO("MainLoop demarre: fixedDt={:.4f}s budget={:.4f}s threads=3+{}",
                      m_FixedDt, m_FrameBudget, m_Workers.ThreadCount());

    while (m_Running.load())
    {
        // Variable dt, clamped against debugger stalls / spiral of death.
        const f32 dt = std::min(std::max(timer.GetDeltaTime(), 0.0f), 0.1f);
        timer.Reset();

        totalTime += dt;
        accumulator += dt;

        FrameState& pending = m_FrameState.Write();
        pending.frameNumber = frame;
        pending.deltaTime   = dt;
        pending.totalTime   = totalTime;
        pending.fixedDt     = m_FixedDt;
        pending.fixedSteps  = 0;

        // Fixed timestep: catch up in steps of m_FixedDt, capped at 4 steps
        // per frame so a single frame can't exceed the 11 ms budget.
        while (accumulator >= m_FixedDt && pending.fixedSteps < 4)
        {
            if (m_SimulationFn)
            {
                m_SimulationFn(m_FixedDt);
            }
            accumulator -= m_FixedDt;
            ++pending.fixedSteps;
        }

        m_FrameState.Commit();
        m_FrameCount.fetch_add(1);
        ++frame;

        // Enforce the frame budget (sleep the remainder of 1/90 s).
        timer.WaitForTargetFramerate(1.0f / std::max(m_FrameBudget, 0.0001f));
    }

    m_Workers.WaitAll();

    if (m_RenderThread.joinable())
    {
        m_RenderThread.join();
    }
    if (m_IoThread.joinable())
    {
        m_IoThread.join();
    }

    m_Workers.Shutdown();

    SYNAPSE_CORE_INFO("MainLoop arrete ({} frames)", m_FrameCount.load());
}

void MainLoop::RequestStop()
{
    m_Running.store(false);
}

void MainLoop::RenderThread()
{
    u64 lastRendered = 0;

    while (m_Running.load())
    {
        const FrameState frame = m_FrameState.Read();
        if (frame.frameNumber != lastRendered)
        {
            lastRendered = frame.frameNumber;
            if (m_RenderFn)
            {
                m_RenderFn(frame);
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
}

void MainLoop::IoThread()
{
    // Reserved for input polling (GLFW/OpenXR events, todo 2.2). Idle for now.
    while (m_Running.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

} // namespace synapse