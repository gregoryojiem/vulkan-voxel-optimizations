#ifndef WORLD_H
#define WORLD_H

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
    uint32_t seed;
};


#endif //WORLD_H
