#pragma once

#include <core/Types.h>

namespace synapse::ecs {

// Runtime type id, stable per component type for the process lifetime.
// Useful for type-erased registries, serialization and debugging.
// Function-local statics in templates share one instance across TUs.
namespace detail {
inline u32& NextComponentId()
{
    static u32 s_NextId = 0;
    return s_NextId;
}
} // namespace detail

template <typename T>
u32 ComponentId()
{
    static const u32 s_Id = detail::NextComponentId()++;
    return s_Id;
}

} // namespace synapse::ecs