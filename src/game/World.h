#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include "Block.h"

class World {
public:
    void init();

    void addBlock(Block block);

    [[nodiscard]] const std::vector<Block>& getBlocks() const;

private:
    std::vector<Block> worldBlocks;
};


#endif //WORLD_H
