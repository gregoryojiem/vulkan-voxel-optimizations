#include "World.h"

#include <iostream>
#include <cmath>

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
        Block::rgbToVec3(255, 0, 0)
    };

    addBlock(pinkBlock);
    addBlock(greenBlock);
    addBlock(purpleBlock);

    chunkManager.fillChunk(glm::vec3(10, 0, 0), greenBlock);
    chunkManager.saveChunkGeometry();
}

static int counter = 0;

void World::mainLoop() {
    counter++;
    if (counter % 100 == -1) {
        double angle = static_cast<double>(counter) / 100.0f * 3.14159f * 2.0f; // Full circle in 200 iterations
        double radius = 1.0f + static_cast<double>(counter) / 200.0f; // Increasing radius

        // Calculate integer block position
        int xPos = static_cast<int>(radius * cos(angle));
        int zPos = static_cast<int>(radius * sin(angle*1000));

        glm::vec3 blockPos = {
            xPos,
            2, // Height of the spiral
            zPos
        };

        Block testBlock = {
            blockPos,
            Block::rgbToVec3(255, 0, 0)
        };
        addBlock(testBlock);
    }
    chunkManager.saveChunkGeometry();
}

void World::addBlock(const Block block) {
    worldBlocks.push_back(block);
    chunkManager.addBlock(block);
}

const std::vector<Block> &World::getBlocks() const {
    return worldBlocks;
}
