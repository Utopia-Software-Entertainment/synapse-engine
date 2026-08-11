#pragma once

#include <core/Types.h>
#include <core/Timer.h>
#include <core/SystemScheduler.h>
#include <core/EventBus.h>

namespace synapse {

class Engine
{
public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void Run();
    void Start();
    void TickFrame();
    bool Running() const { return m_Running; }

private:
    void InitCore();
    void InitModules();
    void Shutdown();

    void ProcessFrame(f32 deltaTime);
    void RenderFrame();

    bool m_Running = false;
    Timer m_Timer;
    SystemScheduler m_Scheduler;
    EventBus m_EventBus;
};

} // namespace synapse
