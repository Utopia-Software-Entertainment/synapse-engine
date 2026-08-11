add_library(synapse_renderer
    Vulkan/VulkanContext.cpp
    Vulkan/VulkanDevice.cpp
    Vulkan/VulkanQueues.cpp
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
