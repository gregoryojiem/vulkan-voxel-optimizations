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
};

class VertexPool {
public:
    static bool newUpdate;

    static void addToVertexPool(const ChunkVertex chunkVertices[MAX_QUADS], uint32_t vertexCount, uint32_t chunkID);

    static std::unordered_map<uint32_t, ChunkMemoryRange> &getOccupiedVertexRanges();

private:
    static std::unordered_map<uint32_t, ChunkMemoryRange> occupiedVertexRanges;
    static std::vector<ChunkMemoryRange> freeVertexRanges;

    static ChunkMemoryRange getAvailableMemoryRange(std::unordered_map<uint32_t, ChunkMemoryRange> &occupiedRanges,
                                                    std::vector<ChunkMemoryRange> &freeMemoryRanges, uint32_t chunkID,
                                                    uint16_t objectCount);

    static void initMemoryRangeInfo(ChunkMemoryRange &rangeToUse, uint32_t objectCount);

    static ChunkMemoryRange splitUpAvailableMemory(std::vector<ChunkMemoryRange> &freeMemoryRanges,
                                                   const ChunkMemoryRange *rangeToSplit, uint32_t requiredSpace);

    static ChunkMemoryRange resizePool();
};

#endif //VERTEXPOOL_H
