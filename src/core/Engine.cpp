#include <core/Engine.h>
#include <core/Logger.h>
#include <core/Memory/MemoryArena.h>

namespace synapse {

Engine::Engine()
{
    Logger::Init();
    SYNAPSE_CORE_INFO("Engine constructor");
}

Engine::~Engine()
{
    Shutdown();
    SYNAPSE_CORE_INFO("Engine destroyed");
}

void Engine::InitCore()
{
    SYNAPSE_CORE_INFO("Initializing core subsystems...");

    m_EventBus.Init();
    m_Scheduler.Init();
    m_Timer.Reset();
}

void Engine::InitModules()
{
    SYNAPSE_CORE_INFO("Initializing engine modules...");
    // Each module init is called by its respective system registration.
    // Order: Platform → Renderer → VR → Physics → Audio → Neural → Voice → Network
}

void Engine::Shutdown()
{
    SYNAPSE_CORE_INFO("Shutting down engine...");
    m_Running = false;
}

void Engine::Run()
{
    Start();

    m_Running = true;
    m_Timer.Reset();

    SYNAPSE_CORE_INFO("Engine main loop started (target: 90 Hz)");

    while (m_Running)
    {
        TickFrame();
    }

    Shutdown();
}

void Engine::Start()
{
    InitCore();
    InitModules();
    m_Running = true;
    m_Timer.Reset();
    SYNAPSE_CORE_INFO("Engine main loop started (target: 90 Hz)");
}

void Engine::TickFrame()
{
    const f32 deltaTime = m_Timer.GetDeltaTime();

    ProcessFrame(deltaTime);
    RenderFrame();

    m_Timer.WaitForTargetFramerate(90.0f);
}

void Engine::ProcessFrame(f32 deltaTime)
{
    m_Scheduler.ExecuteSystems(deltaTime);
}

void Engine::RenderFrame()
{
    // Delegates to Renderer module → Vulkan swapchain + stereo + foveated
}

} // namespace synapse
