#pragma once

#include <core/Types.h>
#include <chrono>
#include <thread>

namespace synapse {

class Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;

    Timer() = default;

    void Reset();
    f32  GetDeltaTime() const;
    void WaitForTargetFramerate(f32 targetFPS) const;

private:
    Clock::time_point m_LastFrame = Clock::now();
};

} // namespace synapse
