#ifndef VERTEXPOOL_H
#define VERTEXPOOL_H
#include <cstdint>
#include <vector>

#include "Vertex.h"
#include "../game/ChunkManager.h"

constexpr size_t VERTEX_SIZE = sizeof(ChunkVertex);
const static std::vector<uint32_t> powersOfTwo = { 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };

static constexpr size_t CHUNK_VERTICES_SIZE = (8 * 8 * 8) * 8;
static constexpr size_t CHUNK_INDICES_SIZE = (8 * 8 * 8) * 64; //there are 36 indices but we round up to 64

extern std::vector<ChunkVertex> globalChunkVertices;
extern std::vector<uint32_t> globalChunkIndices;

struct ChunkMemoryRange{
    uint32_t chunkID;
    uint32_t startPos;
    uint32_t endPos;
    uint32_t offset;
    uint16_t objectCount;
};

class VertexPool {
public:
    static void addToVertexPool(const Chunk& chunk);
    static std::vector<ChunkMemoryRange>& getOccupiedIndexRanges();

private:
    static std::vector<ChunkMemoryRange> occupiedVertexRanges;
    static std::vector<ChunkMemoryRange> occupiedIndexRanges;
    static std::vector<ChunkMemoryRange> freeVertexRanges;
    static std::vector<ChunkMemoryRange> freeIndexRanges;

    static ChunkMemoryRange getAvailableMemoryRange(std::vector<ChunkMemoryRange>& occupiedRanges,
        std::vector<ChunkMemoryRange>& freeMemoryRanges, const Chunk& chunk, uint32_t offset, uint16_t objectCount,
        bool poolType);
    static void initMemoryRangeInfo(ChunkMemoryRange& rangeToUse, bool poolType, uint32_t chunkID,
    uint32_t offset, uint32_t objectCount);
    static ChunkMemoryRange splitUpAvailableMemory(std::vector<ChunkMemoryRange>& freeMemoryRanges,
        const ChunkMemoryRange* rangeToSplit, uint32_t requiredSpace);
    static ChunkMemoryRange resizePool(bool poolType);
};

#endif //VERTEXPOOL_H
