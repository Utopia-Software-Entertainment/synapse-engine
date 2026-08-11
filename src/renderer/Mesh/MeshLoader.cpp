#include <renderer/Mesh/MeshLoader.h>

#include <core/Logger.h>

#include <array>
#include <fstream>
#include <map>
#include <sstream>

namespace synapse {

Mesh MeshLoader::LoadObj(std::string_view filepath)
{
    std::ifstream file(filepath.data());
    if (!file.is_open())
    {
        SYNAPSE_CORE_ERROR("MeshLoader: cannot open '{}'", filepath);
        return {};
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    Mesh mesh;
    std::map<std::array<i32, 3>, u32> indexMap;

    const auto addVertex = [&](i32 vIdx, i32 vtIdx, i32 vnIdx)
    {
        if (vIdx < 0) vIdx = static_cast<i32>(positions.size()) + vIdx;
        if (vtIdx < 0) vtIdx = static_cast<i32>(uvs.size()) + vtIdx;
        if (vnIdx < 0) vnIdx = static_cast<i32>(normals.size()) + vnIdx;

        const std::array<i32, 3> key{vIdx, vtIdx, vnIdx};
        const auto it = indexMap.find(key);
        if (it != indexMap.end())
        {
            mesh.indices.push_back(it->second);
            return;
        }

        MeshVertex vertex{};
        vertex.position = positions[vIdx];
        vertex.color = glm::vec3(1.0f);
        if (vtIdx >= 0) vertex.uv = uvs[vtIdx];
        if (vnIdx >= 0) vertex.normal = normals[vnIdx];

        const u32 newIndex = static_cast<u32>(mesh.vertices.size());
        mesh.vertices.push_back(vertex);
        mesh.indices.push_back(newIndex);
        indexMap[key] = newIndex;
    };

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v")
        {
            glm::vec3 p{};
            iss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (token == "vt")
        {
            glm::vec2 uv{};
            iss >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (token == "vn")
        {
            glm::vec3 n{};
            iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (token == "f")
        {
            std::vector<std::array<i32, 3>> face;
            std::string vertexStr;
            while (iss >> vertexStr)
            {
                std::array<i32, 3> vi{-1, -1, -1};
                const int match = std::sscanf(vertexStr.c_str(), "%d/%d/%d", &vi[0], &vi[1], &vi[2]);
                if (match < 3)
                {
                    vi = {-1, -1, -1};
                    if (std::sscanf(vertexStr.c_str(), "%d//%d", &vi[0], &vi[2]) < 2)
                    {
                        vi = {-1, -1, -1};
                        if (std::sscanf(vertexStr.c_str(), "%d/%d", &vi[0], &vi[1]) < 2)
                        {
                            vi = {-1, -1, -1};
                            std::sscanf(vertexStr.c_str(), "%d", &vi[0]);
                        }
                    }
                }
                face.push_back(vi);
            }

            for (size_t i = 1; i + 1 < face.size(); ++i)
            {
                addVertex(face[0][0], face[0][1], face[0][2]);
                addVertex(face[i][0], face[i][1], face[i][2]);
                addVertex(face[i + 1][0], face[i + 1][1], face[i + 1][2]);
            }
        }
    }

    SYNAPSE_CORE_INFO("Mesh loaded '{}' ({} vertices, {} indices)", filepath,
                      mesh.vertices.size(), mesh.indices.size());
    return mesh;
}

} // namespace synapse