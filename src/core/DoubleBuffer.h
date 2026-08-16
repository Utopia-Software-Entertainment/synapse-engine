#pragma once

#include <core/Types.h>

#include <atomic>

namespace synapse {

// Single-producer / single-consumer double buffer, lock-free via an atomic
// index. The writer mutates Write(), then publishes with Commit(); the reader
// reads the last committed snapshot with Read().
//
// Used by the game loop to hand a consistent FrameState (and later the world
// snapshot) from the simulation thread to the render thread without tearing.
template <typename T>
class DoubleBuffer
{
public:
    DoubleBuffer() = default;
    ~DoubleBuffer() = default;

    DoubleBuffer(const DoubleBuffer&) = delete;
    DoubleBuffer& operator=(const DoubleBuffer&) = delete;

    // Writer side: the buffer that is NOT currently read.
    T& Write()
    {
        return m_Buffers[m_Index.load(std::memory_order_relaxed) ^ 1u];
    }

    // Reader side: the last committed snapshot.
    const T& Read() const
    {
        return m_Buffers[m_Index.load(std::memory_order_acquire)];
    }

    T& Read()
    {
        return m_Buffers[m_Index.load(std::memory_order_acquire)];
    }

    // Publish the written buffer to the reader.
    void Commit()
    {
        m_Index.fetch_xor(1u, std::memory_order_release);
    }

private:
    alignas(64) T m_Buffers[2]{};
    std::atomic<u32> m_Index{0};
};

} // namespace synapse