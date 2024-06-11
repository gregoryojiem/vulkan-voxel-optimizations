#ifndef GRAPHICSUTIL_H
#define GRAPHICSUTIL_H

#include <array>
#include <vector>

#include "../rendering/Vertex.h"

extern void insertBlockVertices(std::vector<ChunkVertex> &chunkVertices, std::array<bool, 6> &facesToDraw,
                                glm::vec3 &blockPos, uint8_t color[4]);

extern void insertBlockIndices(std::vector<uint32_t> &chunkIndices, std::array<bool, 6> &facesToDraw,
                               uint32_t startIndex);

extern std::vector<TexturedVertex> generateTexturedQuad(glm::vec4 quadBounds, glm::vec4 texQuadBounds,
                                                        glm::vec2 startPos);

extern std::vector<uint32_t> generateTexturedQuadIndices(uint32_t startIndex);

#endif //GRAPHICSUTIL_H
