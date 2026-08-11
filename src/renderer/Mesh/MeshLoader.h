#pragma once

namespace synapse {

class MeshLoader
{
public:
    MeshLoader() = default;
    ~MeshLoader() = default;

    MeshLoader(const MeshLoader&) = delete;
    MeshLoader& operator=(const MeshLoader&) = delete;
};

} // namespace synapse
