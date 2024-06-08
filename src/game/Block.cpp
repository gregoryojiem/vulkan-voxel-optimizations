#include "Block.h"

void Block::setColor(Block& block, uint8_t r, uint8_t g, uint8_t b) {
    block.color[0] = r;
    block.color[1] = g;
    block.color[2] = b;
    block.color[3] = 255;
}

void Block::copyBlock(Block& block, const Block& blockInfo) {
    block.position = blockInfo.position;
    setColor(block, blockInfo.color[0], blockInfo.color[1], blockInfo.color[2]);
}

