#include <doctest/doctest.h>

#include <core/ThreadPool.h>

#include <atomic>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

using namespace synapse;

TEST_CASE("ThreadPool: submit returns results")
{
    ThreadPool pool;
    pool.Start(4);

    auto f1 = pool.Submit([]() { return 21 * 2; });
    auto f2 = pool.Submit([]() { return std::string("ok"); });

    CHECK(f1.get() == 42);
    CHECK(f2.get() == "ok");

    pool.Shutdown();
}

TEST_CASE("ThreadPool: many tasks all execute")
{
    ThreadPool pool;
    pool.Start(4);

    constexpr u32 kTasks = 256;
    std::atomic<u32> counter{0};

    std::vector<std::future<void>> futures;
    futures.reserve(kTasks);
    for (u32 i = 0; i < kTasks; ++i)
    {
        futures.push_back(pool.Submit([&counter]() { counter.fetch_add(1); }));
    }
    for (auto& f : futures)
    {
        f.wait();
    }

    CHECK(counter.load() == kTasks);
    pool.Shutdown();
}

TEST_CASE("ThreadPool: parallel-for sums a range")
{
    ThreadPool pool;
    pool.Start(4);

    std::vector<u32> data(1000);
    pool.ParallelFor(static_cast<u32>(data.size()), [&data](u32 start, u32 end) {
        for (u32 i = start; i < end; ++i)
        {
            data[i] = i;
        }
    });

    u64 sum = 0;
    for (u32 v : data)
    {
        sum += v;
    }
    CHECK(sum == 1000ull * 999ull / 2ull);

    pool.Shutdown();
}

TEST_CASE("ThreadPool: shutdown is idempotent")
{
    ThreadPool pool;
    pool.Start(2);
    CHECK(pool.IsRunning());

    pool.Shutdown();
    pool.Shutdown();
    CHECK_FALSE(pool.IsRunning());
}

TEST_CASE("ThreadPool: runs on multiple threads")
{
    ThreadPool pool;
    pool.Start(8);
    CHECK(pool.ThreadCount() == 8);

    std::atomic<u32> distinctThreads{0};
    std::mutex seenMutex;
    std::set<std::thread::id> seen;

    std::vector<std::future<void>> futures;
    constexpr u32 kTasks = 512;
    futures.reserve(kTasks);
    for (u32 i = 0; i < kTasks; ++i)
    {
        futures.push_back(pool.Submit([&]() {
            {
                std::lock_guard<std::mutex> lock(seenMutex);
                seen.insert(std::this_thread::get_id());
            }
            std::this_thread::yield();
        }));
    }
    for (auto& f : futures)
    {
        f.wait();
    }

    CHECK(seen.size() > 1);   // must actually spread work across workers
    (void)distinctThreads;

    pool.Shutdown();
}