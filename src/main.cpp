#include <iostream>

#include "rendering/GameRenderer.h"

int main() {
    GameRenderer renderer;

    try {
        renderer.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
