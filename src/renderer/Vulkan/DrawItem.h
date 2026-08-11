#pragma once

#include <core/Types.h>

#include <glm/glm.hpp>

namespace synapse {

struct DrawItem
{
    glm::mat4 model = glm::mat4(1.0f);
    u32 firstIndex = 0;
    u32 indexCount = 0;
};

} // namespace synapse