#include <iostream>
#include <queue>
#include <thread>

#include "game/World.h"
#include "rendering/MainRenderer.h"

MainRenderer mainRenderer;
World world;

int main() {
    try {
        world.init();
        mainRenderer.init();

        while (!glfwWindowShouldClose(MainRenderer::getWindow())) {
            glfwPollEvents();
            world.mainLoop();
            mainRenderer.draw();
        }

        mainRenderer.cleanup();
    }

    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}