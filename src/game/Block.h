#ifndef BLOCK_H
#define BLOCK_H
#include <glm/vec3.hpp>

struct Block {
    int8_t position[3];
    uint8_t color[4]; //color[3] currently used for debugging
};


#endif //BLOCK_H
