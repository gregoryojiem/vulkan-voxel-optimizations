#include "World.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "../rendering/Vertex.h"
#include "../utility/TimeManager.h"
#include "../utility/TextUtil.h"

World::World() : seed(2) { }

static Block greenBlock = {glm::vec3(0.0f, 0.0f, 0.0f), Block::rgbToVec3(0, 150, 0)};
static Block pinkBlock = {glm::vec3(2.0f, 0.0f, 0.0f), Block::rgbToVec3(255, 174, 201)};
static Block yellowBlock = {glm::vec3(1.0f, 1.0f, 0.0f), Block::rgbToVec3(255, 255, 0)};

void World::generateNoisyTerrain(int range) {
    for (int x = -range; x < range; ++x) {
        for (int z = -range; z < range; ++z) {
            float height = 0.5f + rand() % 100 / 100.0f;
            height += 0.2f * (x / 10.0f) + 0.2f * (z / 10.0f);

            Block block = height > 0.7f ? greenBlock : pinkBlock;
            block.position = glm::vec3(x, static_cast<int>(height * 5.0f), z);
            addBlock(block);
        }
    }
}

void World::generateTerrainFromNoise(int range) {
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
}

void World::init() {
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    addBlock(yellowBlock);

    int range = 1000;
    int numBlocks = range * range * 4;
    std::cout << "Started generating terrain! There are " << TextUtil::getCommaString(numBlocks) << " voxels!\n";

    TimeManager::startTimer("generateTerrain");
    generateTerrainFromNoise(range);
    TimeManager::addTimeToProfiler("generateTerrain", TimeManager::finishTimer("generateTerrain"));

    std::cout << "Started meshing!\n";

    TimeManager::startTimer("meshAllChunks");
    ChunkManager::meshAllChunks();
    TimeManager::addTimeToProfiler("meshAllChunks", TimeManager::finishTimer("meshAllChunks"));

    TimeManager::printAllProfiling();
}

void World::mainLoop() {

}

void World::addBlock(const Block block) {
    worldBlocks.push_back(block);
    chunkManager.addBlock(block);
}

const std::vector<Block> &World::getBlocks() const {
    return worldBlocks;
}
