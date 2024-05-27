#ifndef BLOCK_H
#define BLOCK_H
#include <glm/vec3.hpp>

struct Block {
    glm::vec3 position;
    glm::vec3 color;

    static glm::vec3 rgbToVec3(uint8_t r, uint8_t g, uint8_t b);
};


#endif //BLOCK_H
