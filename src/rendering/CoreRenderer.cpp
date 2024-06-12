#include "CoreRenderer.h"

#include <array>
#include <stdexcept>

#include "vulkan/VulkanUtil.h"

int DEFAULT_WIDTH = 1280;
int DEFAULT_HEIGHT = 720;
const int MAX_FRAMES_IN_FLIGHT = 2;

static std::vector deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

GLFWwindow *CoreRenderer::window;
bool CoreRenderer::windowResized;
VkInstance CoreRenderer::instance;
VkSurfaceKHR CoreRenderer::surface;
VkDevice CoreRenderer::device;
VkPhysicalDevice CoreRenderer::physicalDevice;
VkQueue CoreRenderer::graphicsQueue;
VkQueue CoreRenderer::presentQueue;
SwapChain CoreRenderer::swapChain;
VkRenderPass CoreRenderer::renderPass;
VkCommandPool CoreRenderer::commandPool;
std::vector<VkCommandBuffer> CoreRenderer::commandBuffers;
std::vector<VkSemaphore> CoreRenderer::imageAvailableSemaphores;
std::vector<VkSemaphore> CoreRenderer::renderFinishedSemaphores;
std::vector<VkFence> CoreRenderer::inFlightFences;
uint32_t CoreRenderer::currentFrame;

void CoreRenderer::init() {
    window = initWindow(this, framebufferResizeCallback, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    createInstance(instance);
    createSurface(surface, window, instance);
    physicalDevice = pickPhysicalDevice(instance, surface, deviceExtensions);
    createLogicalDevice(device, graphicsQueue, presentQueue, physicalDevice, surface, deviceExtensions);
    swapChain.init(window, device, physicalDevice, surface);
    createRenderPass(renderPass, swapChain.getImageFormat());
    createCommandPool(commandPool);
    swapChain.createDepthResources();
    swapChain.createFramebuffers(device, renderPass);
    createCommandBuffers(commandBuffers);
    createSyncObjects(imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences);
}

uint32_t CoreRenderer::beginDraw() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain.getSwapChain(), UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChain.recreate(window, device, physicalDevice, surface, renderPass);
        return -1;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    beginRecording(commandBuffer, imageIndex);
    return imageIndex;
}

void CoreRenderer::finishDraw(uint32_t imageIndex) {
    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];
    finishRecording(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowResized) {
        windowResized = false;
        swapChain.recreate(window, device, physicalDevice, surface, renderPass);
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CoreRenderer::beginRecording(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChain.getFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain.getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain.getWidth());
    viewport.height = static_cast<float>(swapChain.getHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CoreRenderer::finishRecording(VkCommandBuffer commandBuffer) {
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CoreRenderer::cleanup() {
    vkDeviceWaitIdle(device);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    swapChain.cleanup(device);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

VkExtent2D CoreRenderer::getExtent() {
    return swapChain.getExtent();
}

uint32_t CoreRenderer::getWindowWidth() {
    return swapChain.getWidth();
}

uint32_t CoreRenderer::getWindowHeight() {
    return swapChain.getHeight();
}

void CoreRenderer::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    windowResized = true;
}
