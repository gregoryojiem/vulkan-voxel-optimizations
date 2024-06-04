#include <iostream>
#include <queue>
#include <thread>

#include "game/World.h"
#include "rendering/GameRenderer.h"
#include "rendering/TextRenderer.h"
#include "rendering/Vertex.h"

GameRenderer renderer;
World world;
TimeManager timeManager;
Camera camera;

int main() {
    try {
        world.init();
        renderer.init();
        camera.init(WIDTH, HEIGHT);

        while (!glfwWindowShouldClose(renderer.window)) {
            glfwPollEvents();
            world.mainLoop();
            camera.update(TimeManager::setDeltaTime());
            renderer.drawFrame();
        }

        vkDeviceWaitIdle(GameRenderer::getDevice());
        renderer.cleanup();
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}