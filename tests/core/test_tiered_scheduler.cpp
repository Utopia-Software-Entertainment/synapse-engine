#include <doctest/doctest.h>

#include <core/Scheduler/TieredScheduler.h>

#include <atomic>
#include <thread>

using namespace synapse;

TEST_CASE("TieredScheduler: les systemes reflex s'executent")
{
    TieredScheduler scheduler;
    std::atomic<int> reflexCount{0};

    scheduler.SetReflexHz(90.0f);
    scheduler.AddReflexSystem([&](f32) { ++reflexCount; }, "reflex_counter");

    scheduler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    scheduler.Tick(0.0f); // laisse l'accumulateur detecter le temps ecoule
    scheduler.Shutdown();

    CHECK(reflexCount.load() >= 1);
}

TEST_CASE("TieredScheduler: les systemes action s'executent")
{
    TieredScheduler scheduler;
    std::atomic<int> actionCount{0};

    scheduler.SetActionHz(60.0f);
    scheduler.AddActionSystem([&](f32) { ++actionCount; }, "action_counter");

    scheduler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    scheduler.Tick(0.0f);
    scheduler.Shutdown();

    CHECK(actionCount.load() >= 1);
}

TEST_CASE("TieredScheduler: les taches cerveau tournent en async et se completent")
{
    TieredScheduler scheduler;
    std::atomic<int> brainCount{0};

    scheduler.AddBrainTask(BrainJobDesc{"brain_counter", [&]() { ++brainCount; }});

    scheduler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    scheduler.Tick(0.0f); // soumet le premier job cerveau
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    scheduler.Shutdown(); // attend la fin du job

    CHECK(brainCount.load() >= 1);
}

TEST_CASE("TieredScheduler: reflex et action cadencés independamment")
{
    TieredScheduler scheduler;
    std::atomic<int> reflexCount{0};
    std::atomic<int> actionCount{0};

    scheduler.SetReflexHz(90.0f);
    scheduler.SetActionHz(30.0f);
    scheduler.AddReflexSystem([&](f32) { ++reflexCount; }, "reflex");
    scheduler.AddActionSystem([&](f32) { ++actionCount; }, "action");

    scheduler.Start();
    // 300 ms réels : reflex (~27 ticks) doit battre action (~9 ticks)
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    scheduler.Tick(0.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(0));
    scheduler.Shutdown();

    CHECK(reflexCount.load() >= actionCount.load());
}