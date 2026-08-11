add_library(synapse_renderer
    Vulkan/VulkanContext.cpp
    Vulkan/VulkanDevice.cpp
    Vulkan/VulkanQueues.cpp
    Vulkan/Renderer.cpp
    Pipeline/Pipeline.cpp
    Pipeline/Pipeline.cpp
    Pipeline/Shader.cpp
    Pipeline/ShaderCompiler.cpp
    Mesh/MeshLoader.cpp
    Mesh/MeshCache.cpp
    Texture/Texture.cpp
    Texture/TextureCache.cpp
    ForwardPlus/ForwardPlusRenderer.cpp
    ForwardPlus/LightCullingPass.cpp
    Culling/CullingSystem.cpp
    Shadow/ShadowPass.cpp
    Shadow/CascadedShadowMap.cpp
    Stereo/StereoRenderer.cpp
    Foveated/FoveatedPass.cpp
    PostProcess/PostProcess.cpp
)

target_link_libraries(synapse_renderer PUBLIC
    synapse_core
    synapse_platform
    Vulkan::Vulkan
    fastgltf
)
target_include_directories(synapse_renderer PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(synapse_renderer PRIVATE
    VulkanMemoryAllocator
)

# --- Shaders : compilation SPIR-V + embed en header C (générés au build) ---
find_program(GLSLC glslc)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc (Vulkan SDK) introuvable — shaders impossible à compiler")
endif()

set(SHADER_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

set(SHADER_LIST
    triangle.vert
    triangle.frag
)

set(SHADER_OUTPUTS "")
foreach(shader ${SHADER_LIST})
    string(REPLACE "." "_" shader_var ${shader})
    string(REGEX REPLACE ".*\\." "" shader_stage ${shader})
    list(APPEND SHADER_OUTPUTS ${SHADER_OUT_DIR}/${shader}.spv ${SHADER_OUT_DIR}/${shader}.h)
    add_custom_command(
        OUTPUT ${SHADER_OUT_DIR}/${shader}.spv ${SHADER_OUT_DIR}/${shader}.h
        COMMAND ${GLSLC} -fshader-stage=${shader_stage} -o ${SHADER_OUT_DIR}/${shader}.spv
                ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${shader}.glsl
        COMMAND ${CMAKE_COMMAND}
                -DINPUT=${SHADER_OUT_DIR}/${shader}.spv
                -DOUTPUT=${SHADER_OUT_DIR}/${shader}.h
                -DVARNAME=${shader_var}
                -P ${CMAKE_SOURCE_DIR}/scripts/embed_shader.cmake
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${shader}.glsl
        VERBATIM
    )
endforeach()

add_custom_target(synapse_shaders DEPENDS ${SHADER_OUTPUTS})
add_dependencies(synapse_renderer synapse_shaders)
target_include_directories(synapse_renderer PRIVATE ${SHADER_OUT_DIR})
