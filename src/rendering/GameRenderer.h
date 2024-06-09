#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "SwapChain.h"
#include "VertexPool.h"
#include "VulkanDebugger.h"
#include "camera/Camera.h"

extern uint32_t WIDTH;
extern uint32_t HEIGHT;
const extern bool DISPLAY_EXTENSIONS;
const extern int MAX_FRAMES_IN_FLIGHT;

class GameRenderer {
public:
    static GLFWwindow* window;
    static VkDevice device;
    static VkPhysicalDevice physicalDevice;

    void init();
    static void drawFrame();
    static void cleanup();

    static uint32_t getWidth();
    static uint32_t getHeight();
    static VkDevice getDevice();

    static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    static void destroyBuffer(const VkBuffer& buffer, const VkDeviceMemory& bufferMemory);

    template <typename VertexType>
    static void createVertexBuffer(VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, VkDeviceSize bufferSize,
        const std::vector<VertexType>& vertices);

    static void createIndexBuffer(VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, VkDeviceSize bufferSize,
        const std::vector<uint32_t>& indices);

    static void updateBuffer(const VkBuffer& buffer,
    const VkBuffer& stagingBuffer, const VkDeviceMemory& stagingBufferMemory, void* newData,
    std::unordered_map<uint32_t, ChunkMemoryRange>& memoryRanges, VkDeviceSize bufferSize, uint32_t objectSize);

    static void resizeBufferCheck();

    static void createUniformBuffers(std::vector<VkBuffer>& uniformBuffers,
        std::vector<VkDeviceMemory>& uniformBuffersMemory, std::vector<void*>& uniformBuffersMapped);

    static VkCommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    static VkImageView createTextureImageView(const VkImage& textureImage);
    static void createTextureSampler(VkSampler& textureSampler);

    static void copyBufferToImage(const VkBuffer& buffer, const VkImage& image, uint32_t width, uint32_t height);
    static void transitionImageLayout(const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);

    static void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, bool includeSamplerDescriptor);

    static void createRenderPass(VkRenderPass& renderPass);

    static void createGraphicsPipeline(VkPipelineLayout& pipelineLayout, VkPipeline& graphicsPipeline,
        const std::vector<char>& vertShaderCode, const std::vector<char>& fragShaderCode,
        const VkVertexInputBindingDescription& bindingDescription,
        const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
        VkDescriptorSetLayout& descriptorSetLayout,
        bool depthEnabled);

    static std::vector<char> readFile(const std::string &filename);

private:
    static VkInstance instance;
    static VulkanDebugger debugger;
    static VkSurfaceKHR surface;

    static VkQueue graphicsQueue;
    static VkQueue presentQueue;

    static SwapChain swapChain;

    static VkRenderPass renderPass;
    static VkPipelineLayout pipelineLayout;
    static VkPipeline graphicsPipeline;

    static VkCommandPool commandPool;
    static std::vector<VkCommandBuffer> commandBuffers;

    static VkDescriptorSetLayout descriptorSetLayout;
    static VkDescriptorPool descriptorPool;
    static std::vector<VkDescriptorSet> descriptorSets;

    static std::vector<VkSemaphore> imageAvailableSemaphores;
    static std::vector<VkSemaphore> renderFinishedSemaphores;
    static std::vector<VkFence> inFlightFences;
    static uint32_t currentFrame;

    static bool framebufferResized;

    static VkBuffer vertexBuffer;
    static VkDeviceMemory vertexBufferMemory;
    static uint32_t vertexMemorySize;
    static VkBuffer vertexStagingBuffer;
    static VkDeviceMemory vertexStagingBufferMemory;

    static VkBuffer indexBuffer;
    static VkDeviceMemory indexBufferMemory;
    static uint32_t indexMemorySize;
    static VkBuffer indexStagingBuffer;
    static VkDeviceMemory indexStagingBufferMemory;

    static VkBuffer drawParamsBuffer;
    static VkDeviceMemory drawParamsBufferMemory;
    static uint32_t drawParamsMemorySize;

    static std::vector<VkBuffer> uniformBuffers;
    static std::vector<VkDeviceMemory> uniformBuffersMemory;
    static std::vector<void*> uniformBuffersMapped;

    static void initVulkan();

    static VkShaderModule createShaderModule(const std::vector<char> &code);

    static void createCommandPool();

    static bool hasStencilComponent(VkFormat format);

    static void createDescriptorPool();

    static void createDescriptorSets();

    static bool createDrawParamsBuffer();

    static void copyBufferRanges(VkBuffer srcBuffer, VkBuffer dstBuffer,
        std::unordered_map<uint32_t, ChunkMemoryRange>& memoryRanges, uint32_t objectSize);

    static void createCommandBuffers();

    static void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    static void createSyncObjects();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

#endif //GAMERENDERER_H
