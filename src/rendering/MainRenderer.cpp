#include "MainRenderer.h"

#include "VulkanDebugger.h"

void MainRenderer::init() {
    coreRenderer.init();
    chunkRenderer.init();
    TextRenderer::init();
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
    chunkRenderer.draw(CoreRenderer::commandBuffers[frame], frame, Camera::ubo);
    TextRenderer::recordDrawCommands(CoreRenderer::commandBuffers[frame], frame);
    coreRenderer.finishDraw(imageIndex);
}

void MainRenderer::cleanup() {
    TextRenderer::cleanup();
    chunkRenderer.cleanup(CoreRenderer::device, MAX_FRAMES_IN_FLIGHT);
    VulkanDebugger::cleanup(CoreRenderer::instance);
    coreRenderer.cleanup();
}

GLFWwindow *MainRenderer::getWindow() {
    return CoreRenderer::window;
}