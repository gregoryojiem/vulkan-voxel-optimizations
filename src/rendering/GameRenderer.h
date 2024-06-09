#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "SwapChain.h"
#include "VulkanDebugger.h"

extern uint32_t DEFAULT_WIDTH;
extern uint32_t DEFAULT_HEIGHT;
const extern int MAX_FRAMES_IN_FLIGHT;

class GameRenderer {
public:
    static GLFWwindow* window;
    static VkDevice device;
    static VkPhysicalDevice physicalDevice;
    static SwapChain swapChain;
    static VkRenderPass renderPass;
    static VkQueue graphicsQueue;
    static VkCommandPool commandPool;

    void init();

    static void drawFrame();

    static void cleanup();

    static void resizeBufferCheck();

    static uint32_t getWindowWidth();

    static uint32_t getWindowHeight();

private:
    static VkInstance instance;
    static VulkanDebugger debugger;
    static VkSurfaceKHR surface;

    static VkQueue presentQueue;

    static VkPipelineLayout pipelineLayout;
    static VkPipeline graphicsPipeline;

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

    static bool createDrawParamsBuffer();

    static void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

#endif //GAMERENDERER_H
