#pragma once

namespace synapse {

class NavMesh
{
public:
    NavMesh() = default;
    ~NavMesh() = default;

    NavMesh(const NavMesh&) = delete;
    NavMesh& operator=(const NavMesh&) = delete;
};

} // namespace synapse
