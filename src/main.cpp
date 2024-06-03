#include <iostream>

#include "game/World.h"
#include "rendering/GameRenderer.h"
#include "rendering/Vertex.h"

int main() {
    GameRenderer renderer;
    World world;
    TimeManager timeManager;
    Camera camera;

    try {
        world.init();
        renderer.init();
        camera.init(WIDTH, HEIGHT);

        while (!glfwWindowShouldClose(renderer.window)) {
            glfwPollEvents();
            world.mainLoop();
            camera.update(timeManager.getDeltaTime());
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
