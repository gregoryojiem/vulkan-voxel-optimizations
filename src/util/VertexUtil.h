#ifndef GRAPHICSUTIL_H
#define GRAPHICSUTIL_H

#include <vector>

#include "../rendering/scene/Vertex.h"
#include "../core/Chunk.h"

extern void insertBlockVertices(std::vector<ChunkVertex> &chunkVertices, const glm::vec3 &blockPos, uint8_t color[4],
                                int face);

extern void insertGreedyQuad(ChunkVertex chunkVertices[MAX_QUADS], uint32_t& vertexStart,
                             const ChunkVertex &vertexInfo,
                             const glm::ivec3 &chunkCorner, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                             uint32_t axis, uint32_t axisPos);

extern std::vector<TexturedVertex> generateTexturedQuad(glm::vec4 quadBounds, glm::vec4 texQuadBounds,
                                                        glm::vec2 startPos);

extern std::vector<uint32_t> generateTexturedQuadIndices(uint32_t startIndex);

#endif //GRAPHICSUTIL_H
