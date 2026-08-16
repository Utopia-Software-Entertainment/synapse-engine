#pragma once

#include <core/Types.h>
#include <entt/entt.hpp>

namespace synapse::ecs {

// EnTT entity as our canonical ECS entity. Opaque strong id; convert with
// ToIndex/FromIndex only when needed (storage keys, serialization).
using Entity = entt::entity;

inline constexpr Entity kNullEntity = entt::null;

inline bool IsValid(Entity entity)          { return entity != entt::null; }
inline u32  ToIndex(Entity entity)          { return entt::to_integral(entity); }
inline Entity FromIndex(u32 index)          { return static_cast<Entity>(index); }

} // namespace synapse::ecs