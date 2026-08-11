add_library(synapse_core
    Logger.cpp
    UUID.cpp
    Timer.cpp
    EventBus.cpp
    SystemScheduler.cpp
    Engine.cpp
    Memory/MemoryArena.cpp
    Memory/PoolAllocator.cpp
    Memory/StackAllocator.cpp
)

target_include_directories(synapse_core PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(synapse_core PUBLIC
    spdlog::spdlog
    EnTT::EnTT
    glm::glm
    nlohmann_json::nlohmann_json
    Taskflow
)
