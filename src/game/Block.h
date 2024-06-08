#ifndef BLOCK_H
#define BLOCK_H
#include <glm/vec3.hpp>

struct Block {
    glm::vec3 position;
    uint8_t color[4];

    static void setColor(Block& block, uint8_t r, uint8_t g, uint8_t b);
    static void copyBlock(Block& block, const Block& blockInfo);
};


#endif //BLOCK_H
