#include "MainRenderer.h"

#include "vulkan/VulkanDebugger.h"
#include "../util/TimeManager.h"

void MainRenderer::init() {
    coreRenderer.init();
    chunkRenderer.init(CoreRenderer::descriptorPool, CoreRenderer::renderPass);
    textRenderer.init(CoreRenderer::descriptorPool, CoreRenderer::renderPass);
    camera.init(CoreRenderer::window, DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void MainRenderer::draw() {
    float deltaTime = TimeManager::setDeltaTime();
    camera.update(deltaTime);
    int imageIndex = CoreRenderer::beginDraw();
    if (imageIndex == -1) {
        return;
    }
    const uint32_t frame = CoreRenderer::currentFrame;
    VkCommandBuffer commandBuffer = CoreRenderer::commandBuffers[frame];
    CoreRenderer::beginRenderPass(commandBuffer, imageIndex);
    chunkRenderer.draw(commandBuffer, frame, Camera::ubo);
    textRenderer.draw(CoreRenderer::device, commandBuffer, frame, TimeManager::queryFPS());
    CoreRenderer::finishDraw(imageIndex);
}

void MainRenderer::cleanup() const {
    textRenderer.cleanup(CoreRenderer::device);
    chunkRenderer.cleanup(CoreRenderer::device, MAX_FRAMES_IN_FLIGHT);
    VulkanDebugger::cleanup(CoreRenderer::instance);
    CoreRenderer::cleanup();
}

GLFWwindow *MainRenderer::getWindow() {
    return CoreRenderer::window;
}
