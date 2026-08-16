#pragma once

#include <core/ECS/Component.h>
#include <core/ECS/Entity.h>
#include <entt/entt.hpp>

namespace synapse::ecs {

// Canonical entity registry — EnTT underneath. This façade keeps game and
// engine code decoupled from EnTT's exact API surface.
using Registry = entt::registry;

inline Entity Spawn(Registry& registry)
{
    return registry.create();
}

inline void Despawn(Registry& registry, Entity entity)
{
    if (IsValid(entity))
    {
        registry.destroy(entity);
    }
}

template <typename T, typename... Args>
T& AddComponent(Registry& registry, Entity entity, Args&&... args)
{
    return registry.emplace<T>(entity, std::forward<Args>(args)...);
}

template <typename T>
T& GetComponent(Registry& registry, Entity entity)
{
    return registry.get<T>(entity);
}

template <typename T>
const T& GetComponent(const Registry& registry, Entity entity)
{
    return registry.get<T>(entity);
}

template <typename T>
bool HasComponent(const Registry& registry, Entity entity)
{
    return registry.all_of<T>(entity);
}

template <typename T>
void RemoveComponent(Registry& registry, Entity entity)
{
    registry.remove<T>(entity);
}

template <typename... Components>
auto View(Registry& registry)
{
    return registry.view<Components...>();
}

template <typename... Components>
auto Group(Registry& registry)
{
    return registry.group<Components...>();
}

} // namespace synapse::ecs