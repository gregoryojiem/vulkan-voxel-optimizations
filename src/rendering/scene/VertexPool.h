#ifndef VERTEXPOOL_H
#define VERTEXPOOL_H
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Vertex.h"
#include "../../core/Chunk.h"

extern std::vector<ChunkVertex> globalChunkVertices;

struct ChunkMemoryRange {
    uint32_t startPos;
    uint32_t endPos;
    uint32_t indexCount;
    uint16_t vertexCount;
    bool savedToVBuffer;
    uint32_t faceOffsets[6];
};

class VertexPool {
public:
    static bool newUpdate;

    static void addToVertexPool(const ChunkVertex chunkVertices[MAX_QUADS], const uint32_t faceOffsets[6],
                                const glm::vec3 &chunkID, uint32_t vertexCount);

    static std::unordered_map<glm::vec3, ChunkMemoryRange> &getOccupiedVertexRanges();

    static uint32_t getIndexCount(ChunkMemoryRange memoryRange, int face);

private:
    static std::unordered_map<glm::vec3, ChunkMemoryRange> occupiedVertexRanges;
    static std::vector<ChunkMemoryRange> freeVertexRanges;

    static ChunkMemoryRange &getAvailableMemoryRange(std::unordered_map<glm::vec3, ChunkMemoryRange> &occupiedRanges,
                                                     std::vector<ChunkMemoryRange> &freeMemoryRanges,
                                                     const glm::vec3 &chunkID, uint32_t requiredVertices);

    static ChunkMemoryRange splitUpAvailableMemory(std::vector<ChunkMemoryRange> &freeMemoryRanges,
                                                   const ChunkMemoryRange *rangeToSplit, uint32_t requiredSpace);

    static ChunkMemoryRange resizePool();

    static uint32_t getAmountOfIndices(uint32_t vertexCount);
};

#endif //VERTEXPOOL_H
