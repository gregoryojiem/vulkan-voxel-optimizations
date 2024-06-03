#include "World.h"

#include <iostream>
#include <cmath>

#include "../rendering/Vertex.h"
#include "FastNoiseLite.h"

World::World() : seed(2) { }

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
        glm::vec3(1.0f, 2.0f, 0.0f),
        Block::rgbToVec3(255, 255, 0)
    };

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    addBlock(purpleBlock);

    int range = 0;
    for (int x = -range; x < range; x++)
    {
        for (int z = -range; z < range; z++)
        {
            float noiseInfo = noise.GetNoise((float)x, (float)z);
            greenBlock.position = {x, (int)(noiseInfo*15), z};
            int test = 50+(int)(noiseInfo*200);
            if (test < 0) {
                test = 0;
            }
            greenBlock.color = Block::rgbToVec3(test, (int)(noiseInfo*100) + 150, test);
            addBlock(greenBlock);
        }
    }

    chunkManager.meshAllChunks();

    return;
    for (int x = -100; x <= 100; ++x) {
        for (int z = -100; z <= 100; ++z) {
            // Calculate a random height with a bit of smoothing
            float height = 0.5f + (rand() % 100) / 100.0f;
            height += 0.2f * (x / 10.0f) + 0.2f * (z / 10.0f); // Add a slight slope

            // Determine block type based on height (higher height = higher ground)
            Block block = (height > 0.7f) ? greenBlock : pinkBlock;
            block.position = glm::vec3(x, static_cast<int>(height * 5.0f), z);
            addBlock(block);
        }
    }

}

static int counter = 0;

void World::mainLoop() {
    //counter++;
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
    //chunkManager.meshAllChunks();
}

void World::addBlock(const Block block) {
    worldBlocks.push_back(block);
    chunkManager.addBlock(block);
}

const std::vector<Block> &World::getBlocks() const {
    return worldBlocks;
}
