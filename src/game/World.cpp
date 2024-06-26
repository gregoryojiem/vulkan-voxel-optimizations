#include "World.h"

#include <iostream>
#include <sstream>

#include "../util/TimeManager.h"
#include "../util/TextUtil.h"

World::World() : seed(2) {
}

uint32_t World::generateTerrainFromNoise(const int range) {
    const int halfRange = range / 2;
    uint32_t blocksGenerated = 0;
    Block terrainBlock{};

    for (int x = -halfRange; x < halfRange; x++) {
        for (int z = -halfRange; z < halfRange; z++) {
            float noiseInfo = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
            noiseInfo = (noiseInfo + 1) / 2;

            const int height = static_cast<int>(noiseInfo * 10);
            for (int y = 0; y <= height; y++) {
                int redBlueColor = (y * 4) / 8 * 8;
                int greenColor = (y * 4 + 50) / 8 * 8;
                const uint8_t redBlueColorClamped = std::clamp(redBlueColor, 0, 255);
                const uint8_t greenColorClamped = std::clamp(greenColor, 0, 255);
                terrainBlock.color[0] = redBlueColorClamped;
                terrainBlock.color[1] = greenColorClamped;
                terrainBlock.color[2] = redBlueColorClamped;
                terrainBlock.initialized = true;
                auto blockPosition = glm::vec3(x, y, z);
                addBlock(blockPosition, terrainBlock);
                blocksGenerated++;
            }
        }
    }

    return blocksGenerated;
}

void World::init() {
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    constexpr int range = 5000;
    std::cout << "Started generating terrain! ";

    TimeManager::startTimer("generateTerrain");
    const uint32_t numBlocksGenerated = generateTerrainFromNoise(range);
    TimeManager::addTimeToProfiler("generateTerrain", TimeManager::finishTimer("generateTerrain"));

    std::cout << "There were " << TextUtil::getCommaString(numBlocksGenerated) << " voxels and " <<
            TextUtil::getCommaString(chunkManager.chunkCount()) << " chunks!\n";

    std::cout << "Started meshing!\n";

    chunkManager.meshAllChunks();

    TimeManager::printAllProfiling();
}

//static int blockCounter = 0;

void World::mainLoop() {
    //blockCounter++;
    //auto position = glm::vec3(blockCounter, 10, 0);
    //addBlock(position, {0, 0, 0, 255, 0, 0, 1});
    //chunkManager.meshAllChunks();
}

void World::addBlock(const glm::vec3 &position, const Block block) {
    chunkManager.addBlock(position, block);
}
