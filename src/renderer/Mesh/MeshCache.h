#pragma once

namespace synapse {

class MeshCache
{
public:
    MeshCache() = default;
    ~MeshCache() = default;

    MeshCache(const MeshCache&) = delete;
    MeshCache& operator=(const MeshCache&) = delete;
};

} // namespace synapse
