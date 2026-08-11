#include <renderer/Vulkan/Renderer.h>
#include <renderer/Pipeline/Pipeline.h>
#include <platform/Window.h>

#include <triangle.vert.h>
#include <triangle.frag.h>

#include <core/Logger.h>

#include <GLFW/glfw3.h>

#include <array>
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <cstring>

namespace synapse {
namespace {

constexpr u32 kMaxFramesInFlight = 2;

#define VK_CHECK(expr)                                                          \
    do                                                                          \
    {                                                                           \
        const VkResult result = (expr);                                         \
        if (result != VK_SUCCESS)                                               \
        {                                                                       \
            SYNAPSE_CORE_CRITICAL("Vulkan error 0x{:x} at {}:{}",               \
                                  static_cast<u32>(result), __FILE__, __LINE__);\
            std::abort();                                                       \
        }                                                                       \
    } while (false)

} // namespace

Renderer::Renderer(Window& window, u32 width, u32 height)
    : m_Window(window), m_Width(width), m_Height(height)
{
    CreateInstance();
    CreateSurface();
    PickPhysicalDevice();
    CreateDevice();
    CreateSwapchain();
    CreateDepthResources();
    CreateRenderPass();
    CreateFramebuffers();
    CreateCommandBuffers();
    CreateSyncObjects();
    CreateTexture();
    CreateDescriptorObjects();
    RecreatePipeline();

    SYNAPSE_CORE_INFO("Vulkan renderer initialized ({}x{}, {} swapchain images)",
                      m_SwapchainExtent.width, m_SwapchainExtent.height,
                      m_SwapchainImages.size());
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(m_Device);

    m_Pipeline.reset();
    CleanupDescriptorObjects();

    vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

    vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
    vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);

    CleanupTexture();

    for (Frame& frame : m_Frames)
    {
        vkDestroySemaphore(m_Device, frame.imageAvailable, nullptr);
        vkDestroySemaphore(m_Device, frame.renderFinished, nullptr);
        vkDestroyFence(m_Device, frame.inFlight, nullptr);
    }

    CleanupSwapchain();
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);

    SYNAPSE_CORE_INFO("Vulkan renderer destroyed");
}

void Renderer::SetClearColor(glm::vec3 color)
{
    m_ClearColor = color;
}

void Renderer::SetViewProjection(glm::mat4 view, glm::mat4 proj)
{
    m_View = view;
    m_Projection = proj;
}

void Renderer::SetDrawItems(std::vector<DrawItem> items)
{
    m_DrawItems = std::move(items);
}

void Renderer::CreateDescriptorObjects()
{
    VkDescriptorSetLayoutBinding bindings[2]{};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;
    VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout));

    VkDescriptorPoolSize poolSizes[2]{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = kMaxFramesInFlight;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = kMaxFramesInFlight;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = kMaxFramesInFlight;
    VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool));

    VkDeviceSize bufferSize = sizeof(CameraUBO);

    std::vector<VkDescriptorSetLayout> layouts(kMaxFramesInFlight, m_DescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = kMaxFramesInFlight;
    allocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> sets(kMaxFramesInFlight);
    VK_CHECK(vkAllocateDescriptorSets(m_Device, &allocInfo, sets.data()));

    for (u32 i = 0; i < kMaxFramesInFlight; ++i)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Frames[i].uboBuffer));

        VkMemoryRequirements memRequirements{};
        vkGetBufferMemoryRequirements(m_Device, m_Frames[i].uboBuffer, &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        u32 memoryType = VK_MAX_MEMORY_TYPES;
        for (u32 t = 0; t < memProperties.memoryTypeCount; ++t)
        {
            if ((memRequirements.memoryTypeBits & (1u << t)) != 0 &&
                (memProperties.memoryTypes[t].propertyFlags &
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) != 0)
            {
                memoryType = t;
                break;
            }
        }

        VkMemoryAllocateInfo allocInfoBuffer{};
        allocInfoBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfoBuffer.allocationSize = memRequirements.size;
        allocInfoBuffer.memoryTypeIndex = memoryType;
        VK_CHECK(vkAllocateMemory(m_Device, &allocInfoBuffer, nullptr, &m_Frames[i].uboMemory));
        VK_CHECK(vkBindBufferMemory(m_Device, m_Frames[i].uboBuffer, m_Frames[i].uboMemory, 0));
        VK_CHECK(vkMapMemory(m_Device, m_Frames[i].uboMemory, 0, bufferSize, 0, &m_Frames[i].uboMapped));

        m_Frames[i].descriptorSet = sets[i];

        VkDescriptorBufferInfo bufferDescriptor{};
        bufferDescriptor.buffer = m_Frames[i].uboBuffer;
        bufferDescriptor.offset = 0;
        bufferDescriptor.range = bufferSize;

        VkDescriptorImageInfo imageDescriptor{};
        imageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageDescriptor.imageView = m_TextureImageView;
        imageDescriptor.sampler = m_TextureSampler;

        VkWriteDescriptorSet writes[2]{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = m_Frames[i].descriptorSet;
        writes[0].dstBinding = 0;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].pBufferInfo = &bufferDescriptor;
        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = m_Frames[i].descriptorSet;
        writes[1].dstBinding = 1;
        writes[1].descriptorCount = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].pImageInfo = &imageDescriptor;
        vkUpdateDescriptorSets(m_Device, 2, writes, 0, nullptr);
    }

    SYNAPSE_CORE_INFO("Descriptor objects created ({} UBO + texture sets)", kMaxFramesInFlight);
}

void Renderer::CleanupDescriptorObjects()
{
    if (m_DescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }
    if (m_DescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }
    for (Frame& frame : m_Frames)
    {
        if (frame.uboMapped != nullptr)
        {
            vkUnmapMemory(m_Device, frame.uboMemory);
            frame.uboMapped = nullptr;
        }
        if (frame.uboBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(m_Device, frame.uboBuffer, nullptr);
            frame.uboBuffer = VK_NULL_HANDLE;
        }
        if (frame.uboMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(m_Device, frame.uboMemory, nullptr);
            frame.uboMemory = VK_NULL_HANDLE;
        }
    }
}

VkBuffer Renderer::CreateDeviceBuffer(u32 size, VkBufferUsageFlags usage, const void* data,
                                      VkDeviceMemory* outMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer = VK_NULL_HANDLE;
    VK_CHECK(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    u32 memoryType = VK_MAX_MEMORY_TYPES;
    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((memRequirements.memoryTypeBits & (1u << i)) != 0 &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
        {
            memoryType = i;
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, outMemory));
    VK_CHECK(vkBindBufferMemory(m_Device, buffer, *outMemory, 0));

    VkBufferCreateInfo stagingInfo{};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.size = size;
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VK_CHECK(vkCreateBuffer(m_Device, &stagingInfo, nullptr, &stagingBuffer));

    vkGetBufferMemoryRequirements(m_Device, stagingBuffer, &memRequirements);

    memoryType = VK_MAX_MEMORY_TYPES;
    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((memRequirements.memoryTypeBits & (1u << i)) != 0 &&
            (memProperties.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) != 0)
        {
            memoryType = i;
            break;
        }
    }

    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &stagingMemory));
    VK_CHECK(vkBindBufferMemory(m_Device, stagingBuffer, stagingMemory, 0));

    void* mapped = nullptr;
    vkMapMemory(m_Device, stagingMemory, 0, size, 0, &mapped);
    std::memcpy(mapped, data, size);
    vkUnmapMemory(m_Device, stagingMemory);

    VkCommandBuffer cmd = BeginSingleTimeCommands();
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(cmd, stagingBuffer, buffer, 1, &copyRegion);
    EndSingleTimeCommands(cmd);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingMemory, nullptr);

    return buffer;
}

VkCommandBuffer Renderer::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
    return cmd;
}

void Renderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(m_GraphicsQueue));

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void Renderer::SetGeometry(const Mesh& mesh)
{
    m_VertexBuffer = CreateDeviceBuffer(static_cast<u32>(mesh.vertices.size() * sizeof(MeshVertex)),
                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                        mesh.vertices.data(), &m_VertexBufferMemory);
    m_IndexBuffer = CreateDeviceBuffer(static_cast<u32>(mesh.indices.size() * sizeof(u16)),
                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                       mesh.indices.data(), &m_IndexBufferMemory);
    m_VertexCount = static_cast<u32>(mesh.vertices.size());
    m_IndexCount = static_cast<u32>(mesh.indices.size());

    SYNAPSE_CORE_INFO("Geometry set ({} vertices, {} indices, device-local)",
                      m_VertexCount, m_IndexCount);
}

void Renderer::CreateTexture()
{
    constexpr u32 kTexWidth = 16;
    constexpr u32 kTexHeight = 16;
    std::array<u8, kTexWidth * kTexHeight * 3> pixels{};
    for (u32 y = 0; y < kTexHeight; ++y)
    {
        for (u32 x = 0; x < kTexWidth; ++x)
        {
            const u8 value = ((x / 2 + y / 2) % 2 == 0) ? 220u : 60u;
            const u32 offset = (y * kTexWidth + x) * 3;
            pixels[offset + 0] = value;
            pixels[offset + 1] = value;
            pixels[offset + 2] = value;
        }
    }

    VkBufferCreateInfo stagingInfo{};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.size = pixels.size();
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VK_CHECK(vkCreateBuffer(m_Device, &stagingInfo, nullptr, &stagingBuffer));

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_Device, stagingBuffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    u32 memoryType = VK_MAX_MEMORY_TYPES;
    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((memRequirements.memoryTypeBits & (1u << i)) != 0 &&
            (memProperties.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) != 0)
        {
            memoryType = i;
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryType;

    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &stagingMemory));
    VK_CHECK(vkBindBufferMemory(m_Device, stagingBuffer, stagingMemory, 0));

    void* mapped = nullptr;
    vkMapMemory(m_Device, stagingMemory, 0, pixels.size(), 0, &mapped);
    std::memcpy(mapped, pixels.data(), pixels.size());
    vkUnmapMemory(m_Device, stagingMemory);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8_UNORM;
    imageInfo.extent = {kTexWidth, kTexHeight, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(m_Device, &imageInfo, nullptr, &m_TextureImage));

    vkGetImageMemoryRequirements(m_Device, m_TextureImage, &memRequirements);

    memoryType = VK_MAX_MEMORY_TYPES;
    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((memRequirements.memoryTypeBits & (1u << i)) != 0 &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
        {
            memoryType = i;
            break;
        }
    }

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_TextureMemory));
    VK_CHECK(vkBindImageMemory(m_Device, m_TextureImage, m_TextureMemory, 0));

    VkCommandBuffer cmd = BeginSingleTimeCommands();

    VkImageMemoryBarrier toTransfer{};
    toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.image = m_TextureImage;
    toTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toTransfer.subresourceRange.levelCount = 1;
    toTransfer.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toTransfer);

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {kTexWidth, kTexHeight, 1};
    vkCmdCopyBufferToImage(cmd, stagingBuffer, m_TextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region);

    VkImageMemoryBarrier toShaderRead{};
    toShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toShaderRead.image = m_TextureImage;
    toShaderRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toShaderRead.subresourceRange.levelCount = 1;
    toShaderRead.subresourceRange.layerCount = 1;
    toShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toShaderRead);

    EndSingleTimeCommands(cmd);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingMemory, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_TextureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_TextureImageView));

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxLod = 1.0f;
    VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_TextureSampler));

    SYNAPSE_CORE_INFO("Texture created ({}x{} damier, device-local)", kTexWidth, kTexHeight);
}

void Renderer::CleanupTexture()
{
    if (m_TextureSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_Device, m_TextureSampler, nullptr);
        m_TextureSampler = VK_NULL_HANDLE;
    }
    if (m_TextureImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_Device, m_TextureImageView, nullptr);
        m_TextureImageView = VK_NULL_HANDLE;
    }
    if (m_TextureImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_Device, m_TextureImage, nullptr);
        m_TextureImage = VK_NULL_HANDLE;
    }
    if (m_TextureMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_Device, m_TextureMemory, nullptr);
        m_TextureMemory = VK_NULL_HANDLE;
    }
}

void Renderer::RecreatePipeline()
{
    m_Pipeline = std::make_unique<Pipeline>(
        m_Device, m_RenderPass, m_SwapchainExtent, m_DescriptorSetLayout,
        std::string_view(reinterpret_cast<const char*>(synapse::triangle_vert_data) +
                            0,
                         synapse::triangle_vert_size),
        std::string_view(reinterpret_cast<const char*>(synapse::triangle_frag_data) +
                            0,
                         synapse::triangle_frag_size));
}

void Renderer::CreateInstance()
{
    u32 glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Synapse Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "Synapse";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
    SYNAPSE_CORE_INFO("Vulkan instance created ({} required extensions)", glfwExtensionCount);
}

void Renderer::CreateSurface()
{
    VK_CHECK(glfwCreateWindowSurface(m_Instance, m_Window.GetHandle(), nullptr, &m_Surface));
    SYNAPSE_CORE_INFO("Vulkan surface created");
}

void Renderer::PickPhysicalDevice()
{
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        SYNAPSE_CORE_CRITICAL("No Vulkan-capable GPU found");
        std::abort();
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (const VkPhysicalDevice& device : devices)
    {
        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (u32 i = 0; i < queueFamilyCount; ++i)
        {
            if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
                if (presentSupport == VK_TRUE)
                {
                    m_PhysicalDevice = device;
                    m_QueueFamilyIndex = i;

                    VkPhysicalDeviceProperties props{};
                    vkGetPhysicalDeviceProperties(device, &props);
                    SYNAPSE_CORE_INFO("GPU selected: {} (queue family {})", props.deviceName, i);
                    return;
                }
            }
        }
    }

    SYNAPSE_CORE_CRITICAL("No suitable GPU with graphics + present queue found");
    std::abort();
}

void Renderer::CreateDevice()
{
    constexpr float kQueuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = m_QueueFamilyIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &kQueuePriority;

    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    createInfo.pEnabledFeatures = &features;

    VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));
    vkGetDeviceQueue(m_Device, m_QueueFamilyIndex, 0, &m_GraphicsQueue);
    SYNAPSE_CORE_INFO("Vulkan device created (VK_KHR_swapchain)");
}

void Renderer::CreateSwapchain()
{
    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, formats.data());

    m_SwapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            break;
        }
    }

    u32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const VkPresentModeKHR& mode : presentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = mode;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &capabilities);

    VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == std::numeric_limits<u32>::max())
    {
        extent.width = std::clamp(m_Width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(m_Height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    m_SwapchainExtent = extent;

    u32 imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_SwapchainFormat;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = m_SwapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain));

    u32 count = 0;
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &count, nullptr);
    m_SwapchainImages.resize(count);
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &count, m_SwapchainImages.data());

    m_SwapchainImageViews.resize(count);
    for (u32 i = 0; i < count; ++i)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_SwapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_SwapchainFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_SwapchainImageViews[i]));
    }

    SYNAPSE_CORE_INFO("Swapchain created ({} images, present mode {})", count,
                      presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? "MAILBOX" : "FIFO");
}

void Renderer::CreateDepthResources()
{
    const VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
                                   VK_FORMAT_D16_UNORM};

    m_DepthFormat = VK_FORMAT_UNDEFINED;
    for (const VkFormat format : candidates)
    {
        VkFormatProperties props{};
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);
        if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
        {
            m_DepthFormat = format;
            break;
        }
    }
    if (m_DepthFormat == VK_FORMAT_UNDEFINED)
    {
        SYNAPSE_CORE_CRITICAL("No supported depth format found");
        std::abort();
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = m_DepthFormat;
    imageInfo.extent = {m_SwapchainExtent.width, m_SwapchainExtent.height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(m_Device, &imageInfo, nullptr, &m_DepthImage));

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(m_Device, m_DepthImage, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    u32 memoryType = VK_MAX_MEMORY_TYPES;
    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((memRequirements.memoryTypeBits & (1u << i)) != 0 &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
        {
            memoryType = i;
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_DepthMemory));
    VK_CHECK(vkBindImageMemory(m_Device, m_DepthImage, m_DepthMemory, 0));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_DepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_DepthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_DepthImageView));
}

void Renderer::CleanupDepthResources()
{
    if (m_DepthImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
        m_DepthImageView = VK_NULL_HANDLE;
    }
    if (m_DepthImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_Device, m_DepthImage, nullptr);
        m_DepthImage = VK_NULL_HANDLE;
    }
    if (m_DepthMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_Device, m_DepthMemory, nullptr);
        m_DepthMemory = VK_NULL_HANDLE;
    }
}

void Renderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_SwapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_DepthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 2;
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_RenderPass));
}

void Renderer::CreateFramebuffers()
{
    m_Framebuffers.resize(m_SwapchainImageViews.size());

    for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
    {
        const VkImageView attachments[] = {m_SwapchainImageViews[i], m_DepthImageView};

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = m_RenderPass;
        createInfo.attachmentCount = 2;
        createInfo.pAttachments = attachments;
        createInfo.width = m_SwapchainExtent.width;
        createInfo.height = m_SwapchainExtent.height;
        createInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(m_Device, &createInfo, nullptr, &m_Framebuffers[i]));
    }
}

void Renderer::CreateCommandBuffers()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_QueueFamilyIndex;

    VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool));

    m_Frames.resize(kMaxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<u32>(m_Frames.size());

    std::vector<VkCommandBuffer> buffers(m_Frames.size());
    VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, buffers.data()));

    for (size_t i = 0; i < m_Frames.size(); ++i)
    {
        m_Frames[i].commandBuffer = buffers[i];
    }
}

void Renderer::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (Frame& frame : m_Frames)
    {
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &frame.imageAvailable));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &frame.renderFinished));
        VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &frame.inFlight));
    }
}

void Renderer::CleanupSwapchain()
{
    for (VkFramebuffer framebuffer : m_Framebuffers)
    {
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    }
    m_Framebuffers.clear();

    for (VkImageView view : m_SwapchainImageViews)
    {
        vkDestroyImageView(m_Device, view, nullptr);
    }
    m_SwapchainImageViews.clear();

    if (m_Swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        m_Swapchain = VK_NULL_HANDLE;
    }

    CleanupDepthResources();
}

void Renderer::Draw()
{
    Frame& frame = m_Frames[m_FrameIndex];

    VK_CHECK(vkWaitForFences(m_Device, 1, &frame.inFlight, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(m_Device, 1, &frame.inFlight));

    u32 imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX,
                                                   frame.imageAvailable, VK_NULL_HANDLE, &imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vkResetFences(m_Device, 1, &frame.inFlight);
        return;
    }
    VK_CHECK(acquireResult);

    const CameraUBO ubo{m_View, m_Projection};
    std::memcpy(frame.uboMapped, &ubo, sizeof(ubo));

    VK_CHECK(vkResetCommandBuffer(frame.commandBuffer, 0));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(frame.commandBuffer, &beginInfo));

    VkClearValue clearValues[2]{};
    clearValues[0].color = {{m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_Framebuffers[imageIndex];
    renderPassInfo.renderArea.extent = m_SwapchainExtent;
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(frame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetHandle());
    vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_Pipeline->GetLayout(), 0, 1, &frame.descriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, &m_VertexBuffer, &offset);
    vkCmdBindIndexBuffer(frame.commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    for (const DrawItem& item : m_DrawItems)
    {
        vkCmdPushConstants(frame.commandBuffer, m_Pipeline->GetLayout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(item.model), &item.model);
        vkCmdDrawIndexed(frame.commandBuffer, item.indexCount, 1, item.firstIndex, 0, 0);
    }

    vkCmdEndRenderPass(frame.commandBuffer);

    VK_CHECK(vkEndCommandBuffer(frame.commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;

    VkSemaphore waitSemaphores[] = {frame.imageAvailable};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderFinished;

    VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, frame.inFlight));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &frame.renderFinished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_Swapchain;
    presentInfo.pImageIndices = &imageIndex;

    VkResult presentResult = vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
    {
        vkDeviceWaitIdle(m_Device);
        CleanupSwapchain();
        CreateSwapchain();
        CreateDepthResources();
        CreateFramebuffers();
        RecreatePipeline();
        SYNAPSE_CORE_WARN("Swapchain recreated after resize");
    }
    else
    {
        VK_CHECK(presentResult);
    }

    m_FrameIndex = (m_FrameIndex + 1) % kMaxFramesInFlight;
}

} // namespace synapse