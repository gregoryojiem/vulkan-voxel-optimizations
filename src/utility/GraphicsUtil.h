#ifndef GRAPHICSUTIL_H
#define GRAPHICSUTIL_H

#include <vector>

#include "../game/Block.h"

struct Vertex;

extern std::vector<Vertex> generateBlockVertices(Block block);

extern std::vector<uint32_t> generateBlockIndices(uint32_t startIndex);

#endif //GRAPHICSUTIL_H
