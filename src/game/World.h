#ifndef WORLD_H
#define WORLD_H

#include <FastNoiseLite.h>
#include <vector>
#include "Block.h"
#include "ChunkManager.h"

class World {
public:
    World();
    void init();
    void mainLoop();
    void addBlock(Block block);

    [[nodiscard]] const std::vector<Block>& getBlocks() const;

private:
    ChunkManager chunkManager;
    std::vector<Block> worldBlocks;
    FastNoiseLite noise;
    uint32_t seed;

    void generateNoisyTerrain(int range);
    void generateTerrainFromNoise(int range);
};


#endif //WORLD_H
