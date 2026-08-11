cmake_minimum_required(VERSION 3.25)
project(synapse-engine
    VERSION 0.1.0
    DESCRIPTION "Neuro-visual VRMMO engine — C++23, Vulkan, OpenXR, ECS"
    LANGUAGES CXX C
)

# ── C++ Standard ──
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Custom CMake modules path ──
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# ── Options ──
option(SYNAPSE_BUILD_TESTS "Build tests" ON)
option(SYNAPSE_BUILD_EDITOR "Build editor tools" ON)
option(SYNAPSE_USE_TRACY "Enable Tracy frame profiling" OFF)

# ── System-level dependencies (SDK) ──
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan SDK found: ${Vulkan_INCLUDE_DIR}")

find_package(OpenXR REQUIRED)
message(STATUS "OpenXR SDK found: ${OpenXR_INCLUDE_DIR}")

# FindOpenXR crée OpenXR::OpenXR uniquement s'il trouve le loader.
# Fallback : cible imported GLOBAL à partir du loader système.
if(NOT TARGET OpenXR::OpenXR)
    find_library(OpenXR_LOADER_LIBRARY NAMES openxr_loader REQUIRED)
    add_library(OpenXR::OpenXR UNKNOWN IMPORTED GLOBAL)
    set_target_properties(OpenXR::OpenXR PROPERTIES
        IMPORTED_LOCATION "${OpenXR_LOADER_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenXR_INCLUDE_DIR}"
    )
endif()

# ── Third-party dependencies ──
add_subdirectory(third_party)

# ── Module libraries ──
add_subdirectory(src/core)
add_subdirectory(src/platform)

add_subdirectory(src/renderer)
add_subdirectory(src/vr)
add_subdirectory(src/physics)
add_subdirectory(src/audio)
add_subdirectory(src/neural)
add_subdirectory(src/voice)
add_subdirectory(src/network)

if(SYNAPSE_BUILD_EDITOR)
    add_subdirectory(src/editor)
endif()

# ── Tests ──
if(SYNAPSE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Engine executable ──
add_executable(synapse_engine
    src/main.cpp
)

target_link_libraries(synapse_engine PRIVATE
    synapse_core
    synapse_platform
    synapse_renderer
    synapse_vr
    synapse_physics
    synapse_audio
    synapse_neural
    synapse_voice
    synapse_network
)

if(SYNAPSE_BUILD_EDITOR)
    target_link_libraries(synapse_engine PRIVATE synapse_editor)
endif()

target_include_directories(synapse_engine PRIVATE src)
