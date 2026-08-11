# Tests par module — les sous-dossiers sont créés au fil de l'eau
# (garde : pas d'erreur de configure tant qu'un module n'a pas de tests).

include(FetchContent)

# --- doctest (framework de test header-only) ---
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(doctest)

foreach(mod core renderer physics network)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${mod}/CMakeLists.txt")
        add_subdirectory(${mod})
    endif()
endforeach()
