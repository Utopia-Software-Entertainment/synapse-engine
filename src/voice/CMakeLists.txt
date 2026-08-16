add_library(synapse_voice
    Whisper/VoiceRecognizer.cpp
    Whisper/WhisperContext.cpp
    Whisper/MicCapture.cpp
    STT/STTPipeline.cpp
    STT/WhisperBackend.cpp
    STT/CrisperWhisperBackend.cpp
    Intent/IntentParser.cpp
    Intent/CommandMatcher.cpp
)

target_link_libraries(synapse_voice PUBLIC
    synapse_core
    whisper
)
target_include_directories(synapse_voice PUBLIC ${CMAKE_SOURCE_DIR}/src)