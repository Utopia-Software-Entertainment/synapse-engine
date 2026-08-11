# Tests par module — les sous-dossiers sont créés au fil de l'eau
# (garde : pas d'erreur de configure tant qu'un module n'a pas de tests).
foreach(mod core renderer physics network)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${mod}/CMakeLists.txt")
        add_subdirectory(${mod})
    endif()
endforeach()