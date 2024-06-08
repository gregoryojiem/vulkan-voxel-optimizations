#include "World.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "../rendering/Vertex.h"
#include "../utility/TimeManager.h"
#include "../utility/TextUtil.h"

World::World() : seed(2) { }

static Block greenBlock = {glm::vec3(0.0f, 0.0f, 0.0f), 0, 150, 0};
static Block pinkBlock = {glm::vec3(2.0f, 0.0f, 0.0f), 255, 174, 201};
static Block yellowBlock = {glm::vec3(1.0f, 3.0f, 0.0f), 255, 255, 0};

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

uint32_t World::generateTerrainFromNoise(int range) {
    const int halfRange = range / 2;
    Block terrainBlock = greenBlock;
    uint32_t blocksGenerated = 0;

    for (int x = -halfRange; x < halfRange; x++) {
        for (int z = -halfRange; z < halfRange; z++) {
            float noiseInfo = noise.GetNoise(static_cast<float>(x), static_cast<float>(z));
            noiseInfo = (noiseInfo + 1) / 2;

            const int height = static_cast<int>(noiseInfo*15);

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

    const int range = 1000;
    std::cout << "Started generating terrain! ";

    TimeManager::startTimer("generateTerrain");
    const uint32_t numBlocksGenerated = generateTerrainFromNoise(range);
    TimeManager::addTimeToProfiler("generateTerrain", TimeManager::finishTimer("generateTerrain"));

    std::cout << "There were " << TextUtil::getCommaString(numBlocksGenerated) << " voxels and " <<
           TextUtil::getCommaString( chunkManager.chunkCount()) << " chunks!\n";

    std::cout << "Started meshing!\n";

    TimeManager::startTimer("meshAllChunks");
    chunkManager.meshAllChunks();
    TimeManager::addTimeToProfiler("meshAllChunks", TimeManager::finishTimer("meshAllChunks"));

    TimeManager::printAllProfiling();
}

void World::mainLoop() {

}

void World::addBlock(const Block block) {
    chunkManager.addBlock(block);
}
