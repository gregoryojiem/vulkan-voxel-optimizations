#ifndef VULKANUTIL_H
#define VULKANUTIL_H

#include <optional>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan_core.h>
#include "GLFW/glfw3.h"

//UTILITY STRUCTS
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 textView;
    alignas(16) glm::mat4 textProj;
};

// OBJECT CREATION FUNCTIONS
GLFWwindow* initWindow(void* userPtr, GLFWframebuffersizefun resizeCallback, int width, int height);

extern void createInstance(VkInstance& instance);

extern void createSurface(VkSurfaceKHR& surface, GLFWwindow* window, const VkInstance& instance);

extern VkPhysicalDevice pickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface,
    std::vector<const char *>& deviceExtensions);

extern void createLogicalDevice(VkDevice& device, VkQueue& graphicsQueue, VkQueue& presentQueue,
    const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface, std::vector<const char *>& deviceExtensions);

extern VkImageView createImageView(const VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags,
                                   const VkDevice& device);

extern VkImageView createTextureImageView(const VkImage& textureImage, const VkDevice& device);

extern void createTextureSampler(VkSampler& textureSampler, const VkDevice& device, const VkPhysicalDevice& physDevice);

extern void createImage(VkImage& image, VkDeviceMemory& imageMemory, const VkDevice& device, const VkPhysicalDevice& physDevice,
                        uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties);

extern void createRenderPass(VkRenderPass& renderPass, const VkDevice& device, const VkPhysicalDevice& physDevice,
    VkFormat imageColorFormat);

extern void createCommandPool(VkCommandPool& commandPool, const VkDevice& device, const VkPhysicalDevice& physDevice,
    const VkSurfaceKHR& surface);

extern void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, const VkDevice& device,
    bool addSampler);

extern void createDescriptorPool(VkDescriptorPool& descriptorPool, uint32_t maxFramesInFlight, const VkDevice& device);

extern void createDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets, uint32_t maxFramesInFlight,
                                 const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout,
                                 const VkDescriptorPool& descriptorPool, const std::vector<VkBuffer>& uniformBuffers);

extern void createGraphicsPipeline(VkPipelineLayout& pipelineLayout, VkPipeline& graphicsPipeline,
    const VkDevice& device, const VkRenderPass& renderPass, VkExtent2D extent,
    const std::string& vertShaderCode, const std::string& fragShaderCode,
    const VkVertexInputBindingDescription& bindingDescription,
    const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
    VkDescriptorSetLayout& descriptorSetLayout,
    bool depthEnabled);

extern VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char> &shaderCode);

extern void createCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, uint32_t maxFramesInFlight,
    const VkDevice& device, const VkCommandPool& commandPool);

extern void createSyncObjects(std::vector<VkSemaphore>& imgAvailableSmphs, std::vector<VkSemaphore>& renderFinishedSmphs,
    std::vector<VkFence>& inFlightFences, uint32_t maxFramesInFlight, const VkDevice& device);

//GENERAL UTILITY FUNCTIONS
extern VkCommandBuffer beginSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool);

extern void endSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
    const VkCommandBuffer& commandBuffer, const VkQueue& graphicsQueue);

extern void transitionImageLayout(const VkImage& image, const VkImageLayout oldLayout, const VkImageLayout newLayout,
    const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

// SUPPORT/QUERY FUNCTIONS
extern QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface);

extern SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface);

extern uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter,
    VkMemoryPropertyFlags properties);

extern VkFormat findDepthFormat(const VkPhysicalDevice& physicalDevice);

static VkFormat findSupportedFormat(const VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features);

static std::vector<const char*> getRequiredExtensions();

static bool isDeviceSuitable(const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface,
    std::vector<const char *>& deviceExtensions);

static bool checkDeviceExtensionSupport(const VkPhysicalDevice& physDevice, std::vector<const char*>& deviceExtensions);

static std::vector<char> readFile(const std::string& filename);

#endif //VULKANUTIL_H
