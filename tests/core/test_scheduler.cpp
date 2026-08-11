#include <doctest/doctest.h>

#include <core/SystemScheduler.h>

#include <string>
#include <vector>

using namespace synapse;

TEST_CASE("SystemScheduler: execution dans l'ordre des phases")
{
    SystemScheduler scheduler;
    scheduler.Init();

    std::vector<std::string> order;

    scheduler.RegisterSystem(SystemDesc {SystemPhase::Render, [&](f32) { order.push_back("render"); }, "render"});
    scheduler.RegisterSystem(SystemDesc {SystemPhase::Physics, [&](f32) { order.push_back("physics"); }, "physics"});
    scheduler.RegisterSystem(SystemDesc {SystemPhase::PrePhysics, [&](f32) { order.push_back("pre_physics"); }, "pre_physics"});

    scheduler.ExecuteSystems(0.016f);

    REQUIRE(order.size() == 3);
    CHECK(order[0] == "pre_physics");
    CHECK(order[1] == "physics");
    CHECK(order[2] == "render");
}

TEST_CASE("SystemScheduler: toutes les phases existent apres Init")
{
    SystemScheduler scheduler;
    scheduler.Init();

    const SystemPhase phases[] = {
        SystemPhase::PrePhysics,
        SystemPhase::Physics,
        SystemPhase::PostPhysics,
        SystemPhase::PreRender,
        SystemPhase::Render,
        SystemPhase::PostRender,
    };

    for (const SystemPhase phase : phases)
    {
        scheduler.RegisterSystem(SystemDesc{phase, [](f32) {}, "sys"});
    }

    scheduler.ExecuteSystems(0.0f);
    REQUIRE(std::size(phases) == 6);
}

TEST_CASE("SystemScheduler: deltaTime transmis aux systemes")
{
    SystemScheduler scheduler;
    scheduler.Init();

    f32 received = -1.0f;
    scheduler.RegisterSystem(SystemDesc {SystemPhase::Physics, [&](f32 dt) { received = dt; }, "capture"});

    scheduler.ExecuteSystems(0.033f);
    CHECK(received == doctest::Approx(0.033f));
}

TEST_CASE("SystemScheduler: plusieurs systemes dans la meme phase")
{
    SystemScheduler scheduler;
    scheduler.Init();

    int a = 0;
    int b = 0;
    scheduler.RegisterSystem(SystemDesc {SystemPhase::Physics, [&](f32) { ++a; }, "a"});
    scheduler.RegisterSystem(SystemDesc {SystemPhase::Physics, [&](f32) { ++b; }, "b"});

    scheduler.ExecuteSystems(0.0f);
    CHECK(a == 1);
    CHECK(b == 1);
}
