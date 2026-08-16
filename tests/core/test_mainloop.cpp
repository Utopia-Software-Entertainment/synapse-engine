#include <doctest/doctest.h>

#include <core/MainLoop.h>

#include <atomic>
#include <chrono>
#include <thread>

using namespace synapse;

TEST_CASE("MainLoop: drives simulation, render and double-buffered snapshots")
{
    MainLoop loop;
    std::atomic<u32> simCalls{0};
    std::atomic<u32> renderCalls{0};
    std::atomic<u64> lastFrameSeen{0};

    loop.SetSimulationFn([&](f32 fixedDt) {
        simCalls.fetch_add(1);
        CHECK(fixedDt == doctest::Approx(1.0f / 90.0f));
    });
    loop.SetRenderFn([&](const FrameState& frame) {
        renderCalls.fetch_add(1);
        lastFrameSeen.store(frame.frameNumber);
    });

    std::thread driver([&loop]() { loop.Run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    loop.RequestStop();
    driver.join();

    CHECK_FALSE(loop.IsRunning());
    CHECK(loop.FrameCount() >= 5);

    // ~22 fixed steps expected in 250 ms at 90 Hz (budget-capped).
    CHECK(simCalls.load() >= 10);
    CHECK(simCalls.load() <= 40);
    CHECK(renderCalls.load() > 0);
    CHECK(lastFrameSeen.load() <= loop.FrameCount() - 1);
    CHECK(lastFrameSeen.load() >= 1);
}

TEST_CASE("MainLoop: worker pool is usable during the loop")
{
    MainLoop loop;
    loop.SetSimulationFn([](f32) {});

    std::atomic<u32> computed{0};
    std::thread driver([&]() {
        loop.Run();
    });

    // Give the loop time to start the pool, then submit async work.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto future = loop.Workers().Submit([&computed]() { return 6 * 7; });
    computed.store(static_cast<u32>(future.get()));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    loop.RequestStop();
    driver.join();

    CHECK(computed.load() == 42);
}

TEST_CASE("MainLoop: run twice is guarded")
{
    MainLoop loop;
    loop.SetSimulationFn([](f32) {});
    loop.SetRenderFn([](const FrameState&) {});

    std::thread driver([&]() { loop.Run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // Second concurrent Run() must return immediately (already running).
    loop.Run();

    loop.RequestStop();
    driver.join();
    CHECK_FALSE(loop.IsRunning());
}