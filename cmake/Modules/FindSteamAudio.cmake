# ── FindSteamAudio.cmake ──
# Finds the Steam Audio SDK installed locally (binary SDK from Valve).
#
# Targets:
#   SteamAudio::SteamAudio  — Interface library: headers + shared lib
#
# Variables:
#   SteamAudio_FOUND        — Set if found
#   SteamAudio_INCLUDE_DIR  — Include path
#   SteamAudio_LIBRARY      — Path to libphonon.so
#   STEAM_AUDIO_SDK         — Hint: set to root of Steam Audio SDK

find_path(SteamAudio_INCLUDE_DIR
    NAMES phonon.h
    PATHS
        "${STEAM_AUDIO_SDK}/include"
        "${CMAKE_SOURCE_DIR}/third_party/steamaudio/include"
        "${CMAKE_SOURCE_DIR}/../steamaudio/include"
)

find_library(SteamAudio_LIBRARY
    NAMES phonon
    PATHS
        "${STEAM_AUDIO_SDK}/lib/linux-x64"
        "${CMAKE_SOURCE_DIR}/third_party/steamaudio/lib/linux-x64"
        "${CMAKE_SOURCE_DIR}/../steamaudio/lib/linux-x64"
)

if(SteamAudio_INCLUDE_DIR AND SteamAudio_LIBRARY)
    set(SteamAudio_FOUND TRUE)
    mark_as_advanced(SteamAudio_INCLUDE_DIR SteamAudio_LIBRARY)
endif()

if(SteamAudio_FOUND AND NOT TARGET SteamAudio::SteamAudio)
    add_library(SteamAudio::SteamAudio UNKNOWN IMPORTED)
    set_target_properties(SteamAudio::SteamAudio PROPERTIES
        IMPORTED_LOCATION "${SteamAudio_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SteamAudio_INCLUDE_DIR}"
    )
endif()

if(NOT SteamAudio_FOUND)
    if(SteamAudio_FIND_REQUIRED)
        message(FATAL_ERROR "Steam Audio SDK not found. Download from "
            "https://github.com/ValveSoftware/steam-audio/releases "
            "and set STEAM_AUDIO_SDK or place in third_party/steamaudio/")
    endif()
endif()
