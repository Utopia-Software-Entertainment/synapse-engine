#include <doctest/doctest.h>

#include <core/Timer.h>

#include <thread>

using namespace synapse;

TEST_CASE("Timer: delta time nul juste apres reset")
{
    Timer timer;
    timer.Reset();
    CHECK(timer.GetDeltaTime() < 0.01f);
}

TEST_CASE("Timer: delta time croit apres attente")
{
    Timer timer;
    timer.Reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    const f32 dt = timer.GetDeltaTime();
    CHECK(dt >= 0.015f);
    CHECK(dt < 1.0f);
}

TEST_CASE("Timer: WaitForTargetFramerate respecte une cible donnee")
{
    Timer timer;
    timer.Reset();
    const auto start = std::chrono::steady_clock::now();
    timer.WaitForTargetFramerate(20.0f);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    CHECK(elapsed >= 30);
    CHECK(elapsed < 200);
}

TEST_CASE("Timer: WaitForTargetFramerate respecte une cible haute")
{
    Timer timer;
    timer.Reset();
    const auto start = std::chrono::steady_clock::now();
    timer.WaitForTargetFramerate(60.0f);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    CHECK(elapsed >= 10);
    CHECK(elapsed < 100);
}
