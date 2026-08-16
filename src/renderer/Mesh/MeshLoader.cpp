#include <renderer/Mesh/MeshLoader.h>
#include <core/Logger.h>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <fstream>
#include <map>
#include <sstream>
#include <filesystem>

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
        if (vtIdx >= 0 && vtIdx < (i32)uvs.size()) vertex.uv = uvs[vtIdx];
        if (vnIdx >= 0 && vnIdx < (i32)normals.size()) vertex.normal = normals[vnIdx];

        const u32 newIndex = static_cast<u32>(mesh.vertices.size());
        mesh.vertices.push_back(vertex);
        mesh.indices.push_back(static_cast<u16>(newIndex));
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
                if (std::sscanf(vertexStr.c_str(), "%d/%d/%d", &vi[0], &vi[1], &vi[2]) == 3) {}
                else if (std::sscanf(vertexStr.c_str(), "%d//%d", &vi[0], &vi[2]) == 2) { vi[1] = -1; }
                else if (std::sscanf(vertexStr.c_str(), "%d/%d", &vi[0], &vi[1]) == 2) { vi[2] = -1; }
                else if (std::sscanf(vertexStr.c_str(), "%d", &vi[0]) == 1) { vi[1] = -1; vi[2] = -1; }

                if (vi[0] > 0) vi[0]--;
                if (vi[1] > 0) vi[1]--;
                if (vi[2] > 0) vi[2]--;
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

Mesh MeshLoader::LoadGltf(std::string_view filepath)
{
    fastgltf::Parser parser;
    fastgltf::GltfDataBuffer buffer;
    if (!buffer.loadFromFile(std::filesystem::path(filepath)))
    {
        SYNAPSE_CORE_ERROR("MeshLoader: Failed to load glTF file '{}'", filepath);
        return {};
    }

    auto asset = parser.loadGltf(&buffer, std::filesystem::path(filepath).parent_path(), fastgltf::Options::None);
    if (asset.error() != fastgltf::Error::None)
    {
        SYNAPSE_CORE_ERROR("MeshLoader: Failed to parse glTF from '{}'", filepath);
        return {};
    }

    Mesh result;
    for (auto& mesh : asset->meshes)
    {
        for (auto& prim : mesh.primitives)
        {
            if (!prim.indicesAccessor.has_value()) continue;
            auto posAttr = prim.findAttribute("POSITION");
            if (posAttr == prim.attributes.end()) continue;

            size_t vertexOffset = result.vertices.size();

            // Positions
            auto& posAccessor = asset->accessors[posAttr->second];
            result.vertices.resize(vertexOffset + posAccessor.count);

            fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), posAccessor, [&](glm::vec3 pos, size_t idx) {
                result.vertices[vertexOffset + idx].position = pos;
                result.vertices[vertexOffset + idx].color = glm::vec3(1.0f);
                result.vertices[vertexOffset + idx].normal = glm::vec3(0, 1, 0);
                result.vertices[vertexOffset + idx].uv = glm::vec2(0, 0);
            });

            // Normals
            if (auto it = prim.findAttribute("NORMAL"); it != prim.attributes.end())
            {
                auto& normAccessor = asset->accessors[it->second];
                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), normAccessor, [&](glm::vec3 norm, size_t idx) {
                    result.vertices[vertexOffset + idx].normal = norm;
                });
            }

            // UVs
            if (auto it = prim.findAttribute("TEXCOORD_0"); it != prim.attributes.end())
            {
                auto& uvAccessor = asset->accessors[it->second];
                fastgltf::iterateAccessorWithIndex<glm::vec2>(asset.get(), uvAccessor, [&](glm::vec2 uv, size_t idx) {
                    result.vertices[vertexOffset + idx].uv = uv;
                });
            }

            // Indices
            auto& indexAccessor = asset->accessors[prim.indicesAccessor.value()];
            size_t indexOffset = result.indices.size();
            result.indices.resize(indexOffset + indexAccessor.count);

            fastgltf::iterateAccessorWithIndex<std::uint32_t>(asset.get(), indexAccessor, [&](std::uint32_t idx, size_t i) {
                result.indices[indexOffset + i] = static_cast<u16>(vertexOffset + idx);
            });
        }
    }

    SYNAPSE_CORE_INFO("glTF Mesh loaded '{}' ({} vertices, {} indices)", filepath,
                      result.vertices.size(), result.indices.size());
    return result;
}

} // namespace synapse
