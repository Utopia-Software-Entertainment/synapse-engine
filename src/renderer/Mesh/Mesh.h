#pragma once

#include <core/Types.h>

#include <glm/glm.hpp>

#include <string_view>
#include <vector>

namespace synapse {

struct MeshVertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct Mesh
{
    std::vector<MeshVertex> vertices;
    std::vector<u16> indices;
};

Mesh CreateCube();
Mesh CreateFloor();

class MeshLoader
{
public:
    static Mesh LoadObj(std::string_view filepath);
};

} // namespace synapse