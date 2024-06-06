#ifndef GRAPHICSUTIL_H
#define GRAPHICSUTIL_H

#include <array>
#include <vector>

#include "../rendering/Vertex.h"
#include "../game/Block.h"

extern void insertBlockVertices(std::vector<ChunkVertex>& chunkVertices, std::array<bool, 6>& facesToDraw, Block* block);

extern void insertBlockIndices(std::vector<uint32_t>& chunkIndices, std::array<bool, 6>& facesToDraw, uint32_t startIndex);

extern std::vector<TexturedVertex> generateTexturedQuad(glm::vec4 quadBounds, glm::vec4 texQuadBounds,
    glm::vec2 startPos, float scale);

extern std::vector<uint32_t> generateTexturedQuadIndices(uint32_t startIndex);

#endif //GRAPHICSUTIL_H
