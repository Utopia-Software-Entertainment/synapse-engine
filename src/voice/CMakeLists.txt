add_library(synapse_voice
    Whisper/VoiceRecognizer.cpp
    Whisper/WhisperContext.cpp
    Whisper/STTPipeline.cpp
    Whisper/MicCapture.cpp
    Intent/IntentParser.cpp
    Intent/CommandMatcher.cpp
)

target_link_libraries(synapse_voice PUBLIC
    synapse_core
    whisper
)
target_include_directories(synapse_voice PUBLIC ${CMAKE_SOURCE_DIR}/src)
