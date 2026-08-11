# ── Third-party dependencies ──
# All fetched via FetchContent for reproducibility.
# System-level deps (Vulkan SDK, OpenXR, OpenAL, etc.) are documented
# in SETUP.md and found via find_package.

include(FetchContent)

# ============================================================
# TIER 1 — Zero-effort, pure header-only or minimal build
# ============================================================

# --- EnTT (ECS) ---
FetchContent_Declare(entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.13.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(entt)

# --- GLM (Math SIMD) ---
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# --- spdlog (Logging) ---
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

# --- GLFW (Windowing) ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# --- nlohmann/json (Serialization) ---
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# --- taskflow (Threading / Task Graph) ---
FetchContent_Declare(taskflow
    GIT_REPOSITORY https://github.com/taskflow/taskflow.git
    GIT_TAG v3.7.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(taskflow)

# --- VMA (Vulkan Memory Allocator) ---
set(VMA_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.1.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(vma)

# ============================================================
# TIER 2 — Libraries with CMake, simple build
# ============================================================

# --- ENet (UDP networking) ---
set(ENET_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG v1.3.18
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(enet)

# --- Jolt Physics ---
set(JOLT_ENABLE_AVX512 OFF CACHE BOOL "" FORCE)
set(JOLT_ENABLE_LTO OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
FetchContent_Declare(jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG v5.1.0
    GIT_SHALLOW TRUE
)
# Jolt v5.x n'a pas de CMakeLists.txt à la racine : le point d'entrée
# est Build/CMakeLists.txt (projet plat, lib Jolt uniquement).
FetchContent_GetProperties(jolt)
if(NOT jolt_POPULATED)
    FetchContent_Populate(jolt)
endif()
add_subdirectory(${jolt_SOURCE_DIR}/Build ${jolt_BINARY_DIR}/Build EXCLUDE_FROM_ALL)

# --- Recast/Detour (NavMesh Pathfinding) ---
set(RECASTNAVIGATION_DEMO OFF CACHE BOOL "" FORCE)
set(RECASTNAVIGATION_TESTS OFF CACHE BOOL "" FORCE)
set(RECASTNAVIGATION_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(recast
    GIT_REPOSITORY https://github.com/recastnavigation/recastnavigation.git
    GIT_TAG v1.6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(recast)

# --- fastgltf (glTF 2.0 asset loader) ---
set(FASTGLTF_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(FASTGLTF_BUILD_M2TGM OFF CACHE BOOL "" FORCE)
FetchContent_Declare(fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf.git
    GIT_TAG v0.7.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fastgltf)

# ============================================================
# TIER 3 — Dear ImGui (needs custom target for Vulkan+GLFW)
# ============================================================

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.9
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)

    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    )

    add_library(imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES=1)
endif()

# ============================================================
# TIER 4 — Tracy Profiler (optional, enables frame profiling)
# ============================================================

option(SYNAPSE_USE_TRACY "Enable Tracy profiling" OFF)

if(SYNAPSE_USE_TRACY)
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    FetchContent_Declare(tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.11.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(tracy)
endif()

# ============================================================
# TIER 5 — whisper.cpp (STT, offline, GPU optional)
# ============================================================

# --- whisper.cpp: Speech-to-text inference ---
# Note: first fetch clones + compiles GGML (~5-10 min on first build).
# Enable WHISPER_VULKAN if you want GPU-accelerated inference.

option(SYNAPSE_WHISPER_VULKAN "Enable Vulkan backend for whisper.cpp" OFF)

set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_NO_AVX OFF CACHE BOOL "" FORCE)

if(SYNAPSE_WHISPER_VULKAN)
    set(WHISPER_VULKAN ON CACHE BOOL "" FORCE)
else()
    set(WHISPER_VULKAN OFF CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(whispercpp
    GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
    GIT_TAG v1.7.1
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    PATCH_COMMAND git -C <SOURCE_DIR> submodule update --init --depth 1 ggml
)
FetchContent_MakeAvailable(whispercpp)

if(TARGET whisper)
    set_target_properties(whisper PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# ============================================================
# TIER 6 — System-level dependencies (find_package)
# ============================================================

# Vulkan, OpenXR, OpenAL, SteamAudio, LSL, BrainFlow
# are found at root / in their respective module CMakeLists files.
