#pragma once

#include <core/Types.h>
#include <core/Timer.h>
#include <core/SystemScheduler.h>
#include <core/EventBus.h>
#include <core/ECS/Registry.h>
#include <core/MainLoop.h>
#include <core/Scene.h>

namespace synapse {

class Engine
{
public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void Run();
    void Start(Renderer& renderer);
    void TickFrame();
    bool Running() const { return m_Running; }

    // World entity registry — shared by all gameplay modules (renderer, physics,
    // VR, voice...). Prefer components over per-module state.
    ecs::Registry& GetRegistry() { return m_Registry; }
    const ecs::Registry& GetRegistry() const { return m_Registry; }

    physics::PhysicsWorld& GetPhysicsWorld() { return *m_PhysicsWorld; }
    Scene& GetScene() { return *m_Scene; }

    // Multi-threaded game loop (todo 1.9).
    MainLoop& GetMainLoop() { return m_MainLoop; }

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
    ecs::Registry m_Registry;
    MainLoop m_MainLoop;
    std::unique_ptr<physics::PhysicsWorld> m_PhysicsWorld;
    std::unique_ptr<Scene> m_Scene;
};

} // namespace synapse
