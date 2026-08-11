#pragma once

namespace synapse {

class SceneSerializer
{
public:
    SceneSerializer() = default;
    ~SceneSerializer() = default;

    SceneSerializer(const SceneSerializer&) = delete;
    SceneSerializer& operator=(const SceneSerializer&) = delete;
};

} // namespace synapse
