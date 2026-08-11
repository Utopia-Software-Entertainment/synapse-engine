add_library(synapse_editor
    ImGui/EditorLayer.cpp
    ImGui/ImGuiVulkanBackend.cpp
    Inspector/EntityInspector.cpp
    Inspector/ComponentRenderer.cpp
    Console/LogConsole.cpp
    Scene/SceneSerializer.cpp
)

target_link_libraries(synapse_editor PUBLIC
    synapse_core
    synapse_renderer
    synapse_vr
    imgui
)
target_include_directories(synapse_editor PUBLIC ${CMAKE_SOURCE_DIR}/src)
