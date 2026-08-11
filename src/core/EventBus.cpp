#include <core/EventBus.h>
#include <core/Logger.h>

namespace synapse {

void EventBus::Init()
{
    SYNAPSE_CORE_INFO("EventBus initialized");
}

void EventBus::Clear()
{
    m_Handlers.clear();
}

} // namespace synapse
