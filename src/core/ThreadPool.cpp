#include <core/ThreadPool.h>

#include <algorithm>
#include <cassert>

namespace synapse {

ThreadPool::~ThreadPool()
{
    Shutdown();
}

void ThreadPool::Start(u32 threadCount)
{
    if (m_Running.exchange(true))
    {
        return;
    }

    if (threadCount == 0)
    {
        threadCount = m_RequestedThreads;
    }
    if (threadCount == 0)
    {
        threadCount = std::max(1u, std::thread::hardware_concurrency());
    }
    m_ThreadCount = threadCount;

    m_Workers.reserve(threadCount);
    for (u32 i = 0; i < threadCount; ++i)
    {
        m_Workers.emplace_back([this]() { Worker(); });
    }
}

void ThreadPool::Shutdown()
{
    if (!m_Running.exchange(false))
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Tasks.clear();
    }
    m_Condition.notify_all();

    for (auto& worker : m_Workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
    m_Workers.clear();
    m_ThreadCount = 0;
}

void ThreadPool::Worker()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Condition.wait(lock, [this]() { return !m_Running.load() || !m_Tasks.empty(); });

            if (!m_Running.load() && m_Tasks.empty())
            {
                return;
            }

            task = std::move(m_Tasks.front());
            m_Tasks.pop_front();
        }

        ++m_Pending;
        task();
        --m_Pending;
    }
}

void ThreadPool::ParallelFor(u32 count, std::function<void(u32 start, u32 end)> body)
{
    if (count == 0)
    {
        return;
    }

    const u32 threads = std::max(1u, m_ThreadCount.load());
    const u32 chunk = std::max(1u, (count + threads - 1) / threads);

    std::vector<std::future<void>> futures;
    futures.reserve(threads);

    for (u32 start = 0; start < count; start += chunk)
    {
        const u32 end = std::min(count, start + chunk);
        futures.push_back(Submit([body, start, end]() { body(start, end); }));
    }

    for (auto& future : futures)
    {
        future.wait();
    }
}

void ThreadPool::WaitAll()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Condition.wait(lock, [this]() { return m_Pending.load() == 0 && m_Tasks.empty(); });
}

} // namespace synapse