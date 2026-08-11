#pragma once

#include <renderer/Mesh/Mesh.h>

#include <string_view>

namespace synapse {

class MeshLoader
{
public:
    static Mesh LoadObj(std::string_view filepath);
};

} // namespace synapse