#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H
#include <vector>
#include <vulkan/vulkan_core.h>

#include "GLFW/glfw3.h"

class SwapChain {
public:
    void init(GLFWwindow *window, const VkDevice &device, const VkPhysicalDevice &physicalDevice,
              const VkSurfaceKHR &surface);

    void createImageViews(const VkDevice &device);

    void createDepthResources(const VkDevice &device, const VkPhysicalDevice &physDevice);

    void createFramebuffers(const VkDevice &device, const VkRenderPass &renderPass);

    void recreate(GLFWwindow *window, const VkDevice &device, const VkPhysicalDevice &physicalDevice,
                  const VkSurfaceKHR &surface, const VkRenderPass &renderPass);

    void cleanup(const VkDevice &device);

    VkSwapchainKHR &getSwapChain();

    std::vector<VkFramebuffer> &getFramebuffers();

    [[nodiscard]] VkFormat getImageFormat() const;

    [[nodiscard]] VkExtent2D getExtent() const;

    [[nodiscard]] uint32_t getWidth() const;

    [[nodiscard]] uint32_t getHeight() const;

private:
    VkSwapchainKHR vkSwapChain{};
    VkExtent2D swapChainExtent{};
    VkFormat swapChainImageFormat{};

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkImage depthImage{};
    VkDeviceMemory depthImageMemory{};
    VkImageView depthImageView{};

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    static VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities);
};


#endif //SWAPCHAIN_H
