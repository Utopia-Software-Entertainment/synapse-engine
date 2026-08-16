add_library(synapse_physics
    Jolt/PhysicsWorld.cpp
    Jolt/RigidBody.cpp
    Jolt/PlayerCharacter.cpp
    Jolt/CollisionShape.cpp
    Jolt/CollisionSystem.cpp
    Jolt/PhysicsQuery.cpp
    Jolt/PhysicsSystem.cpp
    IK/BodyIKSolver.cpp
    IK/FABRIKSolver.cpp
    Navigation/NavMesh.cpp
    Navigation/PathfindingSystem.cpp
)

target_link_libraries(synapse_physics PUBLIC
    synapse_core
    Jolt
    Recast
    Detour
    DetourCrowd
    DetourTileCache
)
target_include_directories(synapse_physics PUBLIC ${CMAKE_SOURCE_DIR}/src)
