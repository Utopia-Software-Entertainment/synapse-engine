add_library(synapse_platform
    Platform.cpp
    Window.cpp
    Input/InputManager.cpp
    Profiler/Profiler.cpp
)

target_link_libraries(synapse_platform PUBLIC
    synapse_core
    glfw
)
target_include_directories(synapse_platform PUBLIC ${CMAKE_SOURCE_DIR}/src)
