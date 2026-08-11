#include <core/Timer.h>

namespace synapse {

void Timer::Reset()
{
    m_LastFrame = Clock::now();
}

f32 Timer::GetDeltaTime() const
{
    auto now = Clock::now();
    auto duration = std::chrono::duration<f32>(now - m_LastFrame);
    return duration.count();
}

void Timer::WaitForTargetFramerate(f32 targetFPS) const
{
    constexpr f32 minSleepMs = 1.0f;
    f32 targetFrameTime = 1.0f / targetFPS;
    f32 elapsed = GetDeltaTime();

    if (elapsed < targetFrameTime)
    {
        f32 sleepTimeMs = (targetFrameTime - elapsed) * 1000.0f - minSleepMs;
        if (sleepTimeMs > 0.0f)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<f32, std::milli>(sleepTimeMs));
        }
    }
}

} // namespace synapse
