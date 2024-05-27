#include <iostream>

#include "game/World.h"
#include "rendering/GameRenderer.h"
#include "rendering/WorldGeometry.h"

int main() {
    GameRenderer renderer;
    World world;

    try {
        world.init();
        renderer.init();

        while (!glfwWindowShouldClose(renderer.window)) {
            glfwPollEvents();
            renderer.drawFrame();
        }

        vkDeviceWaitIdle(renderer.getDevice());
        renderer.cleanup();
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
