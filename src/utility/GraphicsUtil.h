#ifndef GRAPHICSUTIL_H
#define GRAPHICSUTIL_H

#include <vector>

#include "../rendering/Vertex.h"

extern std::vector<ChunkVertex> generateBlockVertices(Block block);

extern std::vector<uint32_t> generateBlockIndices(uint32_t startIndex);

extern std::vector<TexturedVertex> generateTexturedQuad(glm::vec4 quadBounds, glm::vec4 texQuadBounds,
    glm::vec2 startPos, float scale);

extern std::vector<uint32_t> generateTexturedQuadIndices(uint32_t startIndex);

#endif //GRAPHICSUTIL_H
