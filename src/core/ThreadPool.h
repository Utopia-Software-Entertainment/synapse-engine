#pragma once

#include <core/Types.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace synapse {

// Fixed-size worker pool with a FIFO task queue. Used by the game loop for
// parallel system updates and async work (asset loading, network, ...).
// Must be Start()ed before Submit(); Shutdown() is idempotent.
class ThreadPool
{
public:
    explicit ThreadPool(u32 threadCount = 0) : m_RequestedThreads(threadCount) {}
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Start(u32 threadCount = 0);
    void Shutdown();

    // Enqueue a task; returns a future for the result.
    template <typename Fn>
    auto Submit(Fn&& fn) -> std::future<std::invoke_result_t<Fn>>
    {
        using ReturnType = std::invoke_result_t<Fn>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Fn>(fn));
        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Tasks.emplace_back([task]() { (*task)(); });
        }
        m_Condition.notify_one();
        return result;
    }

    // Run `body` over [0, count) split into ThreadCount() chunks in parallel.
    void ParallelFor(u32 count, std::function<void(u32 start, u32 end)> body);

    void WaitAll();
    u32 ThreadCount() const { return m_ThreadCount.load(); }
    bool IsRunning() const { return m_Running.load(); }

private:
    void Worker();

    std::vector<std::thread> m_Workers;
    std::deque<std::function<void()>> m_Tasks;
    std::mutex m_Mutex;
    std::condition_variable m_Condition;

    std::atomic<bool> m_Running{false};
    std::atomic<u32> m_ThreadCount{0};
    std::atomic<u32> m_Pending{0};
    u32 m_RequestedThreads = 0;
};

} // namespace synapse