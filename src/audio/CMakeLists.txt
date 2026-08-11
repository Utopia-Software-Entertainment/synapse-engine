find_package(OpenAL REQUIRED)
find_package(SteamAudio QUIET)

add_library(synapse_audio
    Engine/AudioEngine.cpp
    Engine/AudioSource.cpp
    Engine/AudioSystem.cpp
    HRTF/HRTFProcessor.cpp
    Occlusion/AudioOcclusion.cpp
)

target_link_libraries(synapse_audio PUBLIC
    synapse_core
    OpenAL::OpenAL
)

if(TARGET SteamAudio::SteamAudio)
    target_link_libraries(synapse_audio PRIVATE SteamAudio::SteamAudio)
    target_compile_definitions(synapse_audio PRIVATE SYNAPSE_HAS_STEAM_AUDIO=1)
    message(STATUS "Steam Audio enabled for synapse_audio")
else()
    message(STATUS "Steam Audio not found — audio occlusion/HRTF disabled")
endif()

target_include_directories(synapse_audio PUBLIC ${CMAKE_SOURCE_DIR}/src)
