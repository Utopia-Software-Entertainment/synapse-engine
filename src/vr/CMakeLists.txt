add_library(synapse_vr
    OpenXR/XRSession.cpp
    OpenXR/XRInstance.cpp
    OpenXR/XRHeadTracker.cpp
    OpenXR/XRProjection.cpp
    EyeTracker/XREyeTracker.cpp
    EyeTracker/GazeFilter.cpp
    EyeTracker/KalmanFilter.cpp
    EyeTracker/EyeGazeUpdateSystem.cpp
    RecumbentMode.cpp
    PostureCalibration.cpp
)

target_link_libraries(synapse_vr PUBLIC
    synapse_core
    synapse_renderer
    synapse_physics
    OpenXR::OpenXR
)
target_include_directories(synapse_vr PUBLIC ${CMAKE_SOURCE_DIR}/src)
