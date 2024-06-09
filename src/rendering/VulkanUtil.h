#ifndef VULKANUTIL_H
#define VULKANUTIL_H

#include <optional>
#include <vector>
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

// OBJECT CREATION FUNCTIONS
GLFWwindow* initWindow(void* userPtr, GLFWframebuffersizefun resizeCallback, int width, int height);

extern void createInstance(VkInstance& instance);

extern void createSurface(GLFWwindow* window, const VkInstance& instance, VkSurfaceKHR& surface);

extern VkPhysicalDevice pickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface,
    std::vector<const char *>& deviceExtensions);

extern void createLogicalDevice(VkDevice& device, VkQueue& graphicsQueue, VkQueue& presentQueue,
    const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface, std::vector<const char *>& deviceExtensions);

extern VkImageView createImageView(const VkDevice& device, const VkImage& image, VkFormat format,
    VkImageAspectFlags aspectFlags);

extern void createImage(const VkDevice& device, const VkPhysicalDevice& physDevice, uint32_t width, uint32_t height,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
    VkDeviceMemory& imageMemory);

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

static bool checkDeviceExtensionSupport(const VkPhysicalDevice& physDevice, std::vector<const char *>& deviceExtensions);

#endif //VULKANUTIL_H
