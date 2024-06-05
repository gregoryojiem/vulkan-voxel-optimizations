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


    Block yellowBlock = {
        glm::vec3(1.0f, 1.0f, 0.0f),
        Block::rgbToVec3(255, 255, 0)
    };

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    addBlock(yellowBlock);

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

    range = 0;
    for (int x = -range; x <= range; ++x) {
        for (int z = -range; z <= range; ++z) {
            float height = 0.5f + (rand() % 100) / 100.0f;
            height += 0.2f * (x / 10.0f) + 0.2f * (z / 10.0f);

            Block block = (height > 0.7f) ? greenBlock : pinkBlock;
            block.position = glm::vec3(x, static_cast<int>(height * 5.0f), z);
            addBlock(block);
        }
    }
    chunkManager.meshAllChunks();
}

static int counter = 0;

void World::mainLoop() {
    return;
    counter++;
    if (counter % 100 == 0) {
        glm::vec3 blockPos = {
            counter/100,
            2,
            0
        };

        Block testBlock = {
            blockPos,
            Block::rgbToVec3(255, 0, 0)
        };

        addBlock(testBlock);
        chunkManager.meshAllChunks();
    }
}

void World::addBlock(const Block block) {
    worldBlocks.push_back(block);
    chunkManager.addBlock(block);
}

const std::vector<Block> &World::getBlocks() const {
    return worldBlocks;
}
