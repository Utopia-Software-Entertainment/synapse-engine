#pragma once

#include <renderer/Vulkan/DrawItem.h>

#include <core/Types.h>
#include <renderer/Mesh/Mesh.h>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

struct GLFWwindow;

namespace synapse {

class Pipeline;
class ShadowPass;
class SkyboxPass;
class Window;

class Renderer
{
public:
    Renderer(Window& window, u32 width, u32 height);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Draw();
    void SetClearColor(glm::vec3 color);
    void SetViewProjection(glm::mat4 view, glm::mat4 proj);
    void SetLightViewProjection(glm::mat4 lightViewProj);
    void SetInstanceTransforms(const glm::mat4* transforms, u32 count);
    void SetDrawItems(std::vector<DrawItem> items);
    void SetGeometry(const Mesh& mesh);

private:
    struct Frame
    {
        VkSemaphore imageAvailable = VK_NULL_HANDLE;
        VkSemaphore renderFinished = VK_NULL_HANDLE;
        VkFence inFlight = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkBuffer uboBuffer = VK_NULL_HANDLE;
        VkDeviceMemory uboMemory = VK_NULL_HANDLE;
        VkBuffer instanceBuffer = VK_NULL_HANDLE;
        VkDeviceMemory instanceMemory = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet shadowDescriptorSet = VK_NULL_HANDLE;
        void* uboMapped = nullptr;
        void* instanceMapped = nullptr;
    };

    struct CameraUBO
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 lightVP;
    };

    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateDevice();
    void CreateSwapchain();
    void CreateRenderPass();
    void CreateDepthResources();
    void CreateFramebuffers();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void CreateTexture();
    void CreateDescriptorObjects();
    void RecreatePipeline();
    void CleanupSwapchain();
    void CleanupDepthResources();
    void CleanupDescriptorObjects();
    void CleanupTexture();
    VkBuffer CreateDeviceBuffer(u32 size, VkBufferUsageFlags usage, const void* data,
                                VkDeviceMemory* outMemory);
    void CreateHostVisibleBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* outBuffer,
                                 VkDeviceMemory* outMemory, void** outMapped);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    void RecordCommandBuffer(Frame& frame, u32 imageIndex);

    Window& m_Window;
    u32 m_Width = 0;
    u32 m_Height = 0;
    glm::vec3 m_ClearColor = glm::vec3(0.0f, 0.2f, 0.4f);

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    u32 m_QueueFamilyIndex = 0;

    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    VkFormat m_SwapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_SwapchainExtent{};
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;

    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkImage m_DepthImage = VK_NULL_HANDLE;
    VkDeviceMemory m_DepthMemory = VK_NULL_HANDLE;
    VkImageView m_DepthImageView = VK_NULL_HANDLE;
    VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::vector<Frame> m_Frames;
    u32 m_FrameIndex = 0;

    std::unique_ptr<Pipeline> m_Pipeline;
    std::unique_ptr<ShadowPass> m_ShadowPass;
    std::unique_ptr<SkyboxPass> m_SkyboxPass;
    VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
    u32 m_VertexCount = 0;
    VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;
    u32 m_IndexCount = 0;

    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_ShadowSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkImage m_TextureImage = VK_NULL_HANDLE;
    VkDeviceMemory m_TextureMemory = VK_NULL_HANDLE;
    VkImageView m_TextureImageView = VK_NULL_HANDLE;
    VkSampler m_TextureSampler = VK_NULL_HANDLE;
    std::vector<DrawItem> m_DrawItems;
    std::vector<glm::mat4> m_InstanceTransforms;
    u32 m_InstanceCount = 0;
    glm::mat4 m_View = glm::mat4(1.0f);
    glm::mat4 m_Projection = glm::mat4(1.0f);
    glm::mat4 m_LightVP = glm::mat4(1.0f);
};

} // namespace synapse