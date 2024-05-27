#include "World.h"
#include "../rendering/WorldGeometry.h"

void World::init() {
    Block greenBlock = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        Block::rgbToVec3(0, 150, 0)
    };

    Block pinkBlock = {
        glm::vec3(2.0f, 0.0f, 0.0f),
        Block::rgbToVec3(255, 174, 201)
    };

    Block purpleBlock = {
        glm::vec3(0.0f, 2.0f, 0.0f),
        Block::rgbToVec3(255, 0, 255)
    };

    addBlock(greenBlock);
    addBlock(pinkBlock);
    addBlock(purpleBlock);

}

void World::addBlock(const Block block) {
    worldBlocks.push_back(block);
    tesselateAndAddBlock(block);
}

const std::vector<Block> &World::getBlocks() const {
    return worldBlocks;
}
