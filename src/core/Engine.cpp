#include <core/Engine.h>
#include <core/Logger.h>
#include <core/Memory/MemoryArena.h>
#include <physics/Jolt/PhysicsWorld.h>

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

    // Scene manager needs a physics world (can be created later or passed)
    // For now we create a temporary world or require one.
    // In a real build, PhysicsWorld is a module init.
    // m_Scene = std::make_unique<Scene>(*this, world);

    SYNAPSE_CORE_INFO("Entity registry ready.");
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
    m_MainLoop.RequestStop();
}

void Engine::Run()
{
    // Start() should have been called manually with a renderer.
    // If not, we can't really run.
    if (!m_Scene)
    {
        SYNAPSE_CORE_CRITICAL("Engine::Run() called without calling Start(renderer) first!");
        return;
    }

    // Drive the simulation systems from the multi-threaded loop; rendering
    // happens on a dedicated thread consuming the double-buffered snapshot.
    m_MainLoop.SetSimulationFn([this](f32 fixedDt) { ProcessFrame(fixedDt); });
    m_MainLoop.SetRenderFn([this](const FrameState&) { RenderFrame(); });

    m_MainLoop.Run();
    Shutdown();
}

void Engine::Start(Renderer& renderer)
{
    InitCore();
    InitModules();

    m_PhysicsWorld = std::make_unique<physics::PhysicsWorld>();
    m_Scene = std::make_unique<Scene>(*this, *m_PhysicsWorld, renderer);

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
