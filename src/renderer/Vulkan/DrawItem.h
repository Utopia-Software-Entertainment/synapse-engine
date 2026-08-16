#pragma once

#include <core/Types.h>

namespace synapse {

struct DrawItem
{
    u32 firstIndex = 0;
    u32 indexCount = 0;
    u32 firstInstance = 0;
    u32 instanceCount = 1;
    i32 vertexOffset = 0;
};

} // namespace synapse
