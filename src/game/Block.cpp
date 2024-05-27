#include "Block.h"

glm::vec3 Block::rgbToVec3(uint8_t r, uint8_t g, uint8_t b) {
    return {r/255.0, g/255.0, b/255.0};
}