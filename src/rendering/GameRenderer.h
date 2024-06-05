#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <optional>

#include "camera/Camera.h"

extern uint32_t WIDTH;
extern uint32_t HEIGHT;
const extern bool DISPLAY_EXTENSIONS;
const extern int MAX_FRAMES_IN_FLIGHT;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class GameRenderer {
public:
    GLFWwindow *window;
    static VkDevice device;

    void init();
    void drawFrame();
    void cleanup();

    static uint32_t getWidth();
    static uint32_t getHeight();
    static VkDevice getDevice();

    static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    static void destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory);

    template <typename VertexType>
    static void createVertexBuffer(VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, uint32_t memorySize,
        const std::vector<VertexType>& vertices);

    static void createIndexBuffer(VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, uint32_t memorySize,
        const std::vector<uint32_t>& indices);

    static void createUniformBuffers(std::vector<VkBuffer>& uniformBuffers,
        std::vector<VkDeviceMemory>& uniformBuffersMemory, std::vector<void*>& uniformBuffersMapped);

    static VkCommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    static VkImageView createTextureImageView(VkImage& textureImage);
    static void createTextureSampler(VkSampler& textureSampler);

    static void copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height);
    static void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

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
    static VkDebugUtilsMessengerEXT debugMessenger;
    static VkSurfaceKHR surface;

    static VkPhysicalDevice physicalDevice;

    static VkQueue graphicsQueue;
    static VkQueue presentQueue;

    static VkSwapchainKHR swapChain;
    static std::vector<VkImage> swapChainImages;
    static VkFormat swapChainImageFormat;
    static VkExtent2D swapChainExtent;
    static std::vector<VkImageView> swapChainImageViews;
    static std::vector<VkFramebuffer> swapChainFramebuffers;

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

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertexMemorySize = 0;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    uint32_t indexMemorySize = 0;

    VkBuffer drawParamsBuffer;
    VkDeviceMemory drawParamsBufferMemory;
    uint32_t drawParamsMemorySize = 0;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    void initWindow();

    void initVulkan();

    void createInstance();

    bool checkValidationLayerSupport();

    std::vector<const char *> getRequiredExtensions();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    bool isDeviceSuitable(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();

    void createSwapChain();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    static void createImageViews();

    static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    static VkShaderModule createShaderModule(const std::vector<char> &code);

    void createFramebuffers();

    void cleanupSwapChain();

    void recreateSwapChain();

    void createCommandPool();

    void createDepthResources();

    static VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format);

    static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features);

    void createDescriptorPool();

    void createDescriptorSets();

    bool createDrawParamsBuffer();

    static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createCommandBuffers();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createSyncObjects();

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDebugUtilsMessengerEXT *pDebugMessenger);

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks *pAllocator);
};

#endif //GAMERENDERER_H
