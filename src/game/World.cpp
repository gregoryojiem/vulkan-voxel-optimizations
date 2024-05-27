#include "World.h"
#include "../rendering/WorldGeometry.h"

void World::init() {
    Block greenBlock = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        Block::rgbToVec3(0, 150, 0)
    };

    addBlock(greenBlock);
}

void World::addBlock(const Block block) {
    worldBlocks.push_back(block);
    tesselateAndAddBlock(block);
}

const std::vector<Block> &World::getBlocks() const {
    return worldBlocks;
}
