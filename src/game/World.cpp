#include "World.h"

#include <iostream>
#include <sstream>

#include "../util/TimeManager.h"
#include "../util/TextUtil.h"

World::World() : seed(2) {
}

static Block greenBlock = {glm::vec3(0.0f, 0.0f, 0.0f), 0, 150, 0};

uint32_t World::generateTerrainFromNoise(const int range) {
    const int halfRange = range / 2;
    Block terrainBlock = greenBlock;
    uint32_t blocksGenerated = 0;

    for (int x = -halfRange; x < halfRange; x++) {
        for (int z = -halfRange; z < halfRange; z++) {
            float noiseInfo = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
            noiseInfo = (noiseInfo + 1) / 2;

            const int height = static_cast<int>(noiseInfo * 15);

            for (int y = height; y <= height; y++) {
                int redBlueColor = (y * 4) / 8 * 8;
                int greenColor = (y * 4 + 50) / 8 * 8;
                redBlueColor = std::clamp(redBlueColor, 0, 255);
                greenColor = std::clamp(greenColor, 0, 255);

                terrainBlock.position = {x, y, z};
                Block::setColor(terrainBlock, redBlueColor, greenColor, redBlueColor);

                addBlock(terrainBlock);
                blocksGenerated++;
            }
        }
    }

    return blocksGenerated;
}

void World::init() {
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    //addBlock(yellowBlock);
    //chunkManager.fillChunk(yellowBlock.position, yellowBlock);

    constexpr int range = 1000;
    std::cout << "Started generating terrain! ";

    TimeManager::startTimer("generateTerrain");
    const uint32_t numBlocksGenerated = generateTerrainFromNoise(range);
    TimeManager::addTimeToProfiler("generateTerrain", TimeManager::finishTimer("generateTerrain"));

    std::cout << "There were " << TextUtil::getCommaString(numBlocksGenerated) << " voxels and " <<
            TextUtil::getCommaString(chunkManager.chunkCount()) << " chunks!\n";

    std::cout << "Started meshing!\n";

    TimeManager::startTimer("meshAllChunks");
    chunkManager.meshAllChunks();
    TimeManager::addTimeToProfiler("meshAllChunks", TimeManager::finishTimer("meshAllChunks"));

    TimeManager::printAllProfiling();
}

static int test = 0;

void World::mainLoop() {
    test++;
    addBlock({glm::vec3(test, 10, 0), {255, 0, 0}});
    chunkManager.meshAllChunks();
}

void World::addBlock(const Block block) {
    chunkManager.addBlock(block);
}
