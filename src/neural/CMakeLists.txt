add_library(synapse_neural
    LSL/LSLReceiver.cpp
    EEG/EEGParser.cpp
    EEG/MentalFocusAnalyzer.cpp
    EEG/FFTProcessor.cpp
    EEG/NeuralFilter.cpp
    EMG/JawClenchDetector.cpp
    EMG/BlinkDetector.cpp
    BioMapping/BioInputBridge.cpp
    BioMapping/BioInputMappingSystem.cpp
)

find_package(Threads REQUIRED)

# LSL prefix path
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/third_party/liblsl/install/usr/local")

target_link_libraries(synapse_neural PUBLIC
    synapse_core
    Threads::Threads
)

if(TARGET brainflow::brainflow)
    target_link_libraries(synapse_neural PRIVATE brainflow::brainflow)
    target_compile_definitions(synapse_neural PRIVATE SYNAPSE_HAS_BRAINFLOW=1)
endif()

find_package(LSL QUIET)
if(TARGET LSL::lsl)
    target_link_libraries(synapse_neural PRIVATE LSL::lsl)
    target_compile_definitions(synapse_neural PRIVATE SYNAPSE_HAS_LSL=1)
    message(STATUS "LSL enabled for synapse_neural")
else()
    message(STATUS "LSL not found — BCI streaming disabled")
endif()

target_include_directories(synapse_neural PUBLIC ${CMAKE_SOURCE_DIR}/src)
