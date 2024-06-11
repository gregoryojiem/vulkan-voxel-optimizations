#ifndef VULKANUTIL_H
#define VULKANUTIL_H

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanStructs.h"
#include "GLFW/glfw3.h"

// OBJECT CREATION FUNCTIONS
GLFWwindow *initWindow(void *userPtr, GLFWframebuffersizefun resizeCallback, int width, int height);

extern void createInstance(VkInstance &instance);

extern void createSurface(VkSurfaceKHR &surface, GLFWwindow *window, const VkInstance &instance);

extern VkPhysicalDevice pickPhysicalDevice(const VkInstance &instance, const VkSurfaceKHR &surface,
                                           std::vector<const char *> &deviceExtensions);

extern void createLogicalDevice(VkDevice &device, VkQueue &graphicsQueue, VkQueue &presentQueue,
                                const VkPhysicalDevice &physDevice, const VkSurfaceKHR &surface,
                                std::vector<const char *> &deviceExtensions);

extern VkImageView createImageView(const VkImage &image, VkFormat format, VkImageAspectFlags aspectFlags);

extern VkImageView createTextureImageView(const VkImage &textureImage);

extern void createTextureSampler(VkSampler &textureSampler);

extern void createImage(VkImage &image, VkDeviceMemory &imageMemory, const VkDevice &device,
                        const VkPhysicalDevice &physDevice,
                        uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties);

extern void createRenderPass(VkRenderPass &renderPass,
                             VkFormat imageColorFormat);

extern void createCommandPool(VkCommandPool &commandPool);

extern void createDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout,
                                      bool addSampler);

extern void createDescriptorPool(VkDescriptorPool &descriptorPool, VkDescriptorType type);

extern void createDescriptorSetsUB(std::vector<VkDescriptorSet> &descriptorSets,
                                   const VkDescriptorSetLayout &descriptorSetLayout,
                                   const VkDescriptorPool &descriptorPool, const std::vector<VkBuffer> &uniformBuffers);

void createDescriptorSetsSampler(std::vector<VkDescriptorSet> &descriptorSets,
                                 const VkDescriptorSetLayout &descriptorSetLayout,
                                 const VkDescriptorPool &descriptorPool,
                                 const VkImageView &imageView, const VkSampler &sampler);

extern void createGraphicsPipeline(VkPipelineLayout &pipelineLayout, VkPipeline &graphicsPipeline,
                                   const std::string &vertShaderCode, const std::string &fragShaderCode,
                                   const VkVertexInputBindingDescription &bindingDescription,
                                   const std::vector<VkVertexInputAttributeDescription> &attributeDescriptions,
                                   VkDescriptorSetLayout &descriptorSetLayout,
                                   bool depthEnabled);

extern VkShaderModule createShaderModule(const std::vector<char> &shaderCode);

extern void createCommandBuffers(std::vector<VkCommandBuffer> &commandBuffers);

extern void createSyncObjects(std::vector<VkSemaphore> &imgAvailableSmphs,
                              std::vector<VkSemaphore> &renderFinishedSmphs,
                              std::vector<VkFence> &inFlightFences);

//GENERAL UTILITY FUNCTIONS
extern VkCommandBuffer beginSingleTimeCommands();

extern void endSingleTimeCommands(
    const VkCommandBuffer &commandBuffer);

extern void transitionImageLayout(const VkImage &image, VkImageLayout oldLayout, VkImageLayout newLayout);

// SUPPORT/QUERY FUNCTIONS
extern QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &physDevice, const VkSurfaceKHR &surface);

extern SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &physDevice, const VkSurfaceKHR &surface);

extern uint32_t findMemoryType(const VkPhysicalDevice &physicalDevice, uint32_t typeFilter,
                               VkMemoryPropertyFlags properties);

extern VkFormat findDepthFormat(const VkPhysicalDevice &physicalDevice);

static VkFormat findSupportedFormat(const VkPhysicalDevice &physicalDevice, const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling, VkFormatFeatureFlags features);

static std::vector<const char *> getRequiredExtensions();

static bool isDeviceSuitable(const VkPhysicalDevice &physDevice, const VkSurfaceKHR &surface,
                             std::vector<const char *> &deviceExtensions);

static bool checkDeviceExtensionSupport(const VkPhysicalDevice &physDevice,
                                        std::vector<const char *> &deviceExtensions);

static std::vector<char> readFile(const std::string &filename);

//GETTERS
extern uint32_t getWindowWidth();

extern uint32_t getWindowHeight();

#endif //VULKANUTIL_H