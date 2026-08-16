#pragma once

#include <core/Types.h>
#include <renderer/Mesh/Mesh.h>
#include <string>

namespace synapse::ecs {

struct MeshComponent
{
    Mesh mesh;
    std::string name;

    // Rendering data
    u32 firstIndex = 0;
    u32 indexCount = 0;
    u32 vertexOffset = 0;
};

} // namespace synapse::ecs
