#ifndef CORERENDERER_H
#define CORERENDERER_H

#include <cstdint>
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "ChunkRenderer.h"
#include "Swapchain.h"

extern int DEFAULT_WIDTH;
extern int DEFAULT_HEIGHT;
const extern int MAX_FRAMES_IN_FLIGHT;

class CoreRenderer {
public:
    static GLFWwindow *window;
    static bool windowResized; //todo move to private and remove ::'s
    static VkInstance instance;
    static VkSurfaceKHR surface;
    static VkDevice device;
    static VkPhysicalDevice physicalDevice;
    static VkQueue graphicsQueue;
    static VkQueue presentQueue;
    static SwapChain swapChain;
    static VkRenderPass renderPass;
    static VkCommandPool commandPool;
    static std::vector<VkCommandBuffer> commandBuffers;
    static std::vector<VkSemaphore> imageAvailableSemaphores;
    static std::vector<VkSemaphore> renderFinishedSemaphores;
    static std::vector<VkFence> inFlightFences;
    static uint32_t currentFrame;
    static GameRenderer gameRenderer;

    void init();

    static void drawFrame();

    static void beginRecording(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    static void endRecording(VkCommandBuffer commandBuffer);

    static void cleanup();

    static VkExtent2D getExtent();

    static uint32_t getWindowWidth();

    static uint32_t getWindowHeight();

private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
};


#endif //CORERENDERER_H