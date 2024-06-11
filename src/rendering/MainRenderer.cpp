#include "MainRenderer.h"

#include "VulkanDebugger.h"
#include "../util/TimeManager.h"

void MainRenderer::init() {
    coreRenderer.init();
    chunkRenderer.init();
    textRenderer.init();
    camera.init(CoreRenderer::window, DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void MainRenderer::draw() {
    float deltaTime = TimeManager::setDeltaTime();
    camera.update(deltaTime);
    uint32_t imageIndex = coreRenderer.beginDraw();
    if (imageIndex == -1) {
        return;
    }
    const uint32_t frame = CoreRenderer::currentFrame;
    const VkCommandBuffer commandBuffer = CoreRenderer::commandBuffers[frame];
    chunkRenderer.draw(commandBuffer, frame, Camera::ubo);
    textRenderer.draw(CoreRenderer::device, commandBuffer, frame, TimeManager::queryFPS());
    coreRenderer.finishDraw(imageIndex);
}

void MainRenderer::cleanup() {
    textRenderer.cleanup(CoreRenderer::device);
    chunkRenderer.cleanup(CoreRenderer::device, MAX_FRAMES_IN_FLIGHT);
    VulkanDebugger::cleanup(CoreRenderer::instance);
    coreRenderer.cleanup();
}

GLFWwindow *MainRenderer::getWindow() {
    return CoreRenderer::window;
}
