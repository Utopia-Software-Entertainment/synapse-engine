#pragma once

#include <core/DoubleBuffer.h>
#include <core/ThreadPool.h>
#include <core/Timer.h>
#include <core/Types.h>

#include <atomic>
#include <functional>
#include <thread>

namespace synapse {

// Snapshot exchanged between the simulation thread and the render thread.
struct FrameState
{
    u64 frameNumber = 0;
    f32 deltaTime   = 0.0f;  // variable dt of the frame
    f32 totalTime   = 0.0f;  // time since loop start
    f32 fixedDt     = 0.0f;  // fixed timestep used for simulation
    u32 fixedSteps  = 0;     // fixed steps executed this frame
};

// ─────────────────────────────────────────────────────────────────────────
// MainLoop — game loop multithreadée (todo 1.9).
//
//   Threads :
//     • SIMULATION   (thread principal) : fixed-timestep 1/90s, m_SimulationFn
//     • RENDER       (thread dédié)     : consomme le FrameState double-buffered
//     • IO / INPUT   (thread dédié)     : réservé (GLFW events en 2.2)
//     • ThreadPool 4 workers            : tâches parallèles (ParallelFor, async)
//
//   Budget : chaque frame visée à m_FrameBudget (défaut 1/90 ≈ 11 ms) ; si la
//   simulation déborde, le render consomme la dernière snapshot (pas de blocage).
// ─────────────────────────────────────────────────────────────────────────
class MainLoop
{
public:
    MainLoop() = default;
    ~MainLoop();

    MainLoop(const MainLoop&) = delete;
    MainLoop& operator=(const MainLoop&) = delete;

    void SetSimulationFn(std::function<void(f32 fixedDt)> fn);
    void SetRenderFn(std::function<void(const FrameState& frame)> fn);
    void SetFixedDt(f32 dt)   { m_FixedDt = dt; }
    void SetFrameBudget(f32 seconds) { m_FrameBudget = seconds; }

    // Blocks until RequestStop(). Call from a worker thread for async driving.
    void Run();
    void RequestStop();
    bool IsRunning() const { return m_Running.load(); }

    ThreadPool& Workers() { return m_Workers; }

    u32 FrameCount() const { return m_FrameCount.load(); }

private:
    void RenderThread();
    void IoThread();

    std::function<void(f32)> m_SimulationFn;
    std::function<void(const FrameState&)> m_RenderFn;

    f32 m_FixedDt     = 1.0f / 90.0f;
    f32 m_FrameBudget = 1.0f / 90.0f;

    ThreadPool m_Workers{4};
    DoubleBuffer<FrameState> m_FrameState;

    std::atomic<bool> m_Running{false};
    std::atomic<u32> m_FrameCount{0};

    std::thread m_RenderThread;
    std::thread m_IoThread;
};

} // namespace synapse