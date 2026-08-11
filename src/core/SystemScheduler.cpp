#include <core/SystemScheduler.h>
#include <core/Logger.h>

namespace synapse {

void SystemScheduler::Init()
{
    m_Phases.resize(static_cast<usize>(SystemPhase::PostRender) + 1);
    SYNAPSE_CORE_INFO("SystemScheduler initialized with {} phases", m_Phases.size());
}

void SystemScheduler::RegisterSystem(SystemDesc&& desc)
{
    u8 phaseIdx = static_cast<u8>(desc.phase);
    m_Phases[phaseIdx].push_back(std::move(desc));
}

void SystemScheduler::ExecuteSystems(f32 deltaTime)
{
    for (auto& phase : m_Phases)
    {
        for (auto& system : phase)
        {
            system.fn(deltaTime);
        }
    }
}

} // namespace synapse
