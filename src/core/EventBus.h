#pragma once

#include <core/Types.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>

namespace synapse {

class IEvent { public: virtual ~IEvent() = default; };

using EventHandler = std::function<void(const IEvent&)>;

class EventBus
{
public:
    void Init();

    template<typename T>
    void RegisterHandler(EventHandler handler)
    {
        static_assert(std::is_base_of_v<IEvent, T>, "T must derive from IEvent");
        m_Handlers[std::type_index(typeid(T))].push_back(std::move(handler));
    }

    template<typename T>
    void Emit(const T& event)
    {
        static_assert(std::is_base_of_v<IEvent, T>, "T must derive from IEvent");
        auto it = m_Handlers.find(std::type_index(typeid(T)));
        if (it != m_Handlers.end())
        {
            for (auto& handler : it->second)
            {
                handler(event);
            }
        }
    }

    void Clear();

private:
    std::unordered_map<std::type_index, std::vector<EventHandler>> m_Handlers;
};

} // namespace synapse
