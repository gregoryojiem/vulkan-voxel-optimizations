#ifndef WORLD_H
#define WORLD_H

#include <FastNoiseLite.h>
#include "Block.h"
#include "ChunkManager.h"

class World {
public:
    World();

    void init();

    void mainLoop();

    void addBlock(const glm::vec3& position, Block block);

private:
    ChunkManager chunkManager;
    FastNoiseLite noise;
    uint32_t seed;

    uint32_t generateTerrainFromNoise(int range);
};


#endif //WORLD_H
