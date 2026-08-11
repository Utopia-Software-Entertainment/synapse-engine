# ── FindOpenXR.cmake ──
# Finds the OpenXR SDK (loader + headers) installed on the system.
#
# Targets:
#   OpenXR::OpenXR    — Interface library: headers + loader link
#
# Variables:
#   OpenXR_FOUND        — Set if found
#   OpenXR_INCLUDE_DIR  — Include path
#   OpenXR_LIBRARY      — Path to libopenxr_loader

find_path(OpenXR_INCLUDE_DIR
    NAMES openxr/openxr.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/openxr/include
        $ENV{OPENXR_SDK}/include
)

find_library(OpenXR_LIBRARY
    NAMES openxr_loader openxr
    PATHS
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/local/lib
        /opt/openxr/lib
        $ENV{OPENXR_SDK}/lib
)

if(OpenXR_INCLUDE_DIR AND OpenXR_LIBRARY)
    set(OpenXR_FOUND TRUE)
    mark_as_advanced(OpenXR_INCLUDE_DIR OpenXR_LIBRARY)
endif()

if(OpenXR_FOUND AND NOT TARGET OpenXR::OpenXR)
    add_library(OpenXR::OpenXR UNKNOWN IMPORTED)
    set_target_properties(OpenXR::OpenXR PROPERTIES
        IMPORTED_LOCATION "${OpenXR_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenXR_INCLUDE_DIR}"
    )
endif()

if(NOT OpenXR_FOUND)
    if(OpenXR_FIND_REQUIRED)
        message(FATAL_ERROR "OpenXR SDK not found. Install it or set OPENXR_SDK.")
    endif()
endif()
