#include "GameRenderer.h"


#include <iostream>
#include <cstdint>
#include <algorithm>
#include <fstream>

#include "VulkanBufferUtil.h"
#include "VulkanUtil.h"
#include "camera/Camera.h"
#include "VertexPool.h"
#include "TextRenderer.h"

uint32_t DEFAULT_WIDTH = 1280;
uint32_t DEFAULT_HEIGHT = 720;
const int MAX_FRAMES_IN_FLIGHT = 2;

GLFWwindow* GameRenderer::window;
VkPhysicalDevice GameRenderer::physicalDevice;
VkDevice GameRenderer::device;

VkInstance GameRenderer::instance;
VkSurfaceKHR GameRenderer::surface;

VkQueue GameRenderer::graphicsQueue;
VkQueue GameRenderer::presentQueue;

SwapChain GameRenderer::swapChain;

VkRenderPass GameRenderer::renderPass;
VkPipelineLayout GameRenderer::pipelineLayout;
VkPipeline GameRenderer::graphicsPipeline;

VkCommandPool GameRenderer::commandPool;
std::vector<VkCommandBuffer> GameRenderer::commandBuffers;

VkDescriptorSetLayout GameRenderer::descriptorSetLayout;
VkDescriptorPool GameRenderer::descriptorPool;
std::vector<VkDescriptorSet> GameRenderer::descriptorSets;

std::vector<VkSemaphore> GameRenderer::imageAvailableSemaphores;
std::vector<VkSemaphore> GameRenderer::renderFinishedSemaphores;
std::vector<VkFence> GameRenderer::inFlightFences;
uint32_t GameRenderer::currentFrame = 0;

bool GameRenderer::framebufferResized = false;

VkBuffer GameRenderer::vertexBuffer;
VkDeviceMemory GameRenderer::vertexBufferMemory;
uint32_t GameRenderer::vertexMemorySize = 0;
VkBuffer GameRenderer::vertexStagingBuffer;
VkDeviceMemory GameRenderer::vertexStagingBufferMemory;

VkBuffer GameRenderer::indexBuffer;
VkDeviceMemory GameRenderer::indexBufferMemory;
uint32_t GameRenderer::indexMemorySize = 0;
VkBuffer GameRenderer::indexStagingBuffer;
VkDeviceMemory GameRenderer::indexStagingBufferMemory;

VkBuffer GameRenderer::drawParamsBuffer;
VkDeviceMemory GameRenderer::drawParamsBufferMemory;
uint32_t GameRenderer::drawParamsMemorySize = 0;

std::vector<VkBuffer> GameRenderer::uniformBuffers;
std::vector<VkDeviceMemory> GameRenderer::uniformBuffersMemory;
std::vector<void*> GameRenderer::uniformBuffersMapped;

const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

std::vector deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}

void GameRenderer::init() {
    window = initWindow(this, framebufferResizeCallback, static_cast<int>(DEFAULT_WIDTH), static_cast<int>(DEFAULT_HEIGHT));
    initVulkan();
    InputHandler::init(window);
    TextRenderer::init();
}


void GameRenderer::initVulkan() {
    createInstance(instance);

    createSurface(surface, window, instance);

    physicalDevice = pickPhysicalDevice(instance, surface, deviceExtensions);

    createLogicalDevice(device, graphicsQueue, presentQueue, physicalDevice, surface, deviceExtensions);

    swapChain.init(window, device, physicalDevice, surface);

    createRenderPass(renderPass, device, physicalDevice, swapChain.getImageFormat());

    createCommandPool(commandPool, device, physicalDevice, surface);

    createUniformBuffers(uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, MAX_FRAMES_IN_FLIGHT,
        device, physicalDevice);

    createDescriptorSetLayout(descriptorSetLayout, device, false);

    createDescriptorPool(descriptorPool, MAX_FRAMES_IN_FLIGHT, device);

    createDescriptorSets(descriptorSets, MAX_FRAMES_IN_FLIGHT, device, descriptorSetLayout, descriptorPool,
                         uniformBuffers);

    createGraphicsPipeline(
        pipelineLayout, graphicsPipeline, device, renderPass, swapChain.getExtent(),
        "../src/rendering/shaders/vert.spv",
        "../src/rendering/shaders/frag.spv",
        ChunkVertex::getBindingDescription(), ChunkVertex::getAttributeDescriptions(), descriptorSetLayout,
        true);

    swapChain.createDepthResources(device, physicalDevice);
    swapChain.createFramebuffers(device, renderPass);

    vertexMemorySize = sizeof(globalChunkVertices[0]) * globalChunkVertices.size();
    indexMemorySize = sizeof(globalChunkIndices[0]) * globalChunkIndices.size();
    createVertexBuffer(vertexBuffer, vertexBufferMemory, vertexMemorySize, globalChunkVertices, device,
        physicalDevice, commandPool, graphicsQueue);
    createIndexBuffer(indexBuffer, indexBufferMemory, indexMemorySize, globalChunkIndices, device, physicalDevice,
        commandPool, graphicsQueue);

    createBuffer(vertexStagingBuffer, vertexStagingBufferMemory, vertexMemorySize,
        device, physicalDevice,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    createBuffer(indexStagingBuffer, indexStagingBufferMemory, indexMemorySize,
    device, physicalDevice,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    createCommandBuffers(commandBuffers, MAX_FRAMES_IN_FLIGHT, device, commandPool);

    createSyncObjects(imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences, MAX_FRAMES_IN_FLIGHT, device);
}

static std::vector<float> frameTimes;
static float timeBetweenDisplay = 0.25f;
static float accumulatedTime = 0;

void printFPS() {
    float deltaTime = TimeManager::getDeltaTime();
    frameTimes.push_back(deltaTime);
    accumulatedTime += deltaTime;

    if (accumulatedTime < timeBetweenDisplay) {
        return;
    }

    accumulatedTime = 0;

    float totalTime = 0;
    for (const auto& time : frameTimes) {
        totalTime += time;
    }

    float fps = static_cast<float>(frameTimes.size()) / totalTime;

    frameTimes = { };

    std::stringstream ss;
    ss << "fps: " << fps;
    std::string fpsString = ss.str();
    TextRenderer::addText(fpsString, glm::vec2(5.0f, GameRenderer::getWindowHeight()-24), 24.0f, 1);

    accumulatedTime = 0;
}

void GameRenderer::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain.getSwapChain(), UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChain.recreate(window, device, physicalDevice, surface, renderPass);
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame],  0);

    resizeBufferCheck();

    if (VertexPool::newUpdate) {
        updateBuffer(vertexBuffer, vertexStagingBuffer, vertexStagingBufferMemory, globalChunkVertices.data(),
                     vertexMemorySize, sizeof(ChunkVertex), device, VertexPool::getOccupiedVertexRanges(),
                     commandPool, graphicsQueue);

        updateBuffer(indexBuffer, indexStagingBuffer, indexStagingBufferMemory,  globalChunkIndices.data(),
                     indexMemorySize, sizeof(globalChunkIndices[0]), device, VertexPool::getOccupiedIndexRanges(),
                     commandPool, graphicsQueue);

        VertexPool::newUpdate = false;
    }

    printFPS();

    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    UniformBufferObject ubo = Camera::ubo;
    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

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

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        swapChain.recreate(window, device, physicalDevice, surface, renderPass);
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void GameRenderer::cleanup() {
    vkDeviceWaitIdle(device);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);

        destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i], device);
    }

    swapChain.cleanup(device);

    destroyBuffer(indexBuffer, indexBufferMemory, device);
    destroyBuffer(indexStagingBuffer, indexStagingBufferMemory, device);
    destroyBuffer(vertexBuffer, vertexBufferMemory, device);
    destroyBuffer(vertexStagingBuffer, vertexStagingBufferMemory, device);
    destroyBuffer(drawParamsBuffer, drawParamsBufferMemory, device);

    TextRenderer::cleanup();

    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

    if (VulkanDebugger::enabled()) {
        VulkanDebugger::cleanup(instance);
    }

    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GameRenderer::resizeBufferCheck() {
    uint32_t verticesSize = sizeof(globalChunkVertices[0]) * globalChunkVertices.size();
    uint32_t indicesSize = sizeof(globalChunkIndices[0]) * globalChunkIndices.size();

    if (verticesSize > vertexMemorySize) {
        vertexMemorySize = verticesSize;
        destroyBuffer(vertexBuffer, vertexBufferMemory, device);
        createVertexBuffer(vertexBuffer, vertexBufferMemory, vertexMemorySize, globalChunkVertices,
            device, physicalDevice, commandPool, graphicsQueue);

        destroyBuffer(vertexStagingBuffer, vertexStagingBufferMemory, device);
        createBuffer(vertexStagingBuffer, vertexStagingBufferMemory, vertexMemorySize,
            device, physicalDevice,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    if (indicesSize > indexMemorySize) {
        indexMemorySize = indicesSize;
        destroyBuffer(indexBuffer, indexBufferMemory, device);
        createIndexBuffer(indexBuffer, indexBufferMemory, indexMemorySize, globalChunkIndices, device, physicalDevice,
            commandPool, graphicsQueue);

        destroyBuffer(indexStagingBuffer, indexStagingBufferMemory, device);
        createBuffer(indexStagingBuffer, indexStagingBufferMemory, indexMemorySize,
            device, physicalDevice,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

bool GameRenderer::createDrawParamsBuffer() {
    const VkDeviceSize bufferSize = sizeof(VkDrawIndexedIndirectCommand) * VertexPool::getOccupiedIndexRanges().size();

    if (bufferSize == 0) {
        return false;
    }

    if (bufferSize != drawParamsMemorySize) { //todo benchmark with fixed size buffers here
        destroyBuffer(drawParamsBuffer, drawParamsBufferMemory, device);
        createBuffer(drawParamsBuffer, drawParamsBufferMemory, bufferSize,
            device, physicalDevice,
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        drawParamsMemorySize = bufferSize;
    }

    void* data;
    vkMapMemory(device, drawParamsBufferMemory, 0, bufferSize, 0, &data);

    uint32_t commandIndex = 0;
    for (auto& [chunkID, memoryRange] : VertexPool::getOccupiedIndexRanges()) {
        VkDrawIndexedIndirectCommand command;
        command.indexCount = memoryRange.objectCount;
        command.instanceCount = 1;
        command.firstIndex = memoryRange.startPos;
        command.vertexOffset = static_cast<int32_t>(memoryRange.offset);
        command.firstInstance = 0;

        memcpy(static_cast<char*>(data) + commandIndex * sizeof(VkDrawIndexedIndirectCommand),
                    &command,
                    sizeof(VkDrawIndexedIndirectCommand));

        commandIndex++;
    }

    vkUnmapMemory(device, drawParamsBufferMemory);
    return true;
}

void GameRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // draw chunk geometry
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
        0, 1, &descriptorSets[currentFrame], 0, nullptr);

    if (createDrawParamsBuffer()) {
        uint32_t drawCount = VertexPool::getOccupiedIndexRanges().size();
        vkCmdDrawIndexedIndirect(commandBuffer, drawParamsBuffer, 0, drawCount, sizeof(VkDrawIndexedIndirectCommand));
    }

    // draw text and ui
    TextRenderer::recordDrawCommands(commandBuffer, currentFrame);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

uint32_t GameRenderer::getWindowWidth() {
    return swapChain.getWidth();
}

uint32_t GameRenderer::getWindowHeight() {
    return swapChain.getHeight();
}

void GameRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    framebufferResized = true;
}