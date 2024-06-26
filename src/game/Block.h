#ifndef BLOCK_H
#define BLOCK_H
#include <glm/vec3.hpp>

struct Block {
    int8_t position[3];
    uint8_t color[3];
    bool initialized;
};


#endif //BLOCK_H
