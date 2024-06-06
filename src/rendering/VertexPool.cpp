#include "VertexPool.h"

std::vector<ChunkVertex> globalChunkVertices(CHUNK_VERTICES_SIZE);
std::vector<uint32_t> globalChunkIndices(CHUNK_INDICES_SIZE);

std::vector<ChunkMemoryRange> VertexPool::occupiedVertexRanges;
std::vector<ChunkMemoryRange> VertexPool::occupiedIndexRanges;
std::vector<ChunkMemoryRange> VertexPool::freeVertexRanges = {{0, 0, CHUNK_VERTICES_SIZE}};
std::vector<ChunkMemoryRange> VertexPool::freeIndexRanges = {{0, 0, CHUNK_INDICES_SIZE}};

void VertexPool::addToVertexPool(const Chunk& chunk) {
    ChunkMemoryRange vertexRangeToUse = getAvailableMemoryRange(occupiedVertexRanges, freeVertexRanges, chunk,
        0, chunk.vertices.size(), false);
    ChunkMemoryRange indexRangeToUse = getAvailableMemoryRange(occupiedIndexRanges, freeIndexRanges, chunk,
        vertexRangeToUse.startPos, chunk.indices.size(), true);

    std::copy(chunk.vertices.begin(), chunk.vertices.end(), globalChunkVertices.begin() + vertexRangeToUse.startPos);
    std::copy(chunk.indices.begin(), chunk.indices.end(), globalChunkIndices.begin() + indexRangeToUse.startPos);
}

std::vector<ChunkMemoryRange>& VertexPool::getOccupiedVertexRanges() {
    return occupiedVertexRanges;
}

std::vector<ChunkMemoryRange>& VertexPool::getOccupiedIndexRanges() {
    return occupiedIndexRanges;
}

ChunkMemoryRange VertexPool::getAvailableMemoryRange(std::vector<ChunkMemoryRange>& occupiedRanges,
    std::vector<ChunkMemoryRange>& freeMemoryRanges, const Chunk& chunk,  uint32_t offset, const uint16_t objectCount,
    const bool poolType) {
    const auto requiredObjects  = *std::lower_bound(powersOfTwo.begin(), powersOfTwo.end(), objectCount);

    // if the chunk has already been allocated memory, and it is enough space to save the new mesh, save it
    // otherwise, free up the chunk's occupied range and move on
    for (auto it = occupiedRanges.begin(); it != occupiedRanges.end(); ++it) {
        if (it->chunkID == chunk.ID && it->endPos - it->startPos >= requiredObjects) {
            initMemoryRangeInfo(*it, poolType, chunk.ID, offset, objectCount);
            return *it;
        }

        if (it->chunkID == chunk.ID) {
            it->chunkID = 0;
            freeMemoryRanges.push_back(*it);
            occupiedRanges.erase(it);
            break;
        }
    }

    // look for a suitable existing range with the correct size
    // if no existing range can be found, find an available one that can be split up
    uint32_t bestSplittableRangeSize = UINT32_MAX;
    auto eraseRangeIterator = freeMemoryRanges.end();

    ChunkMemoryRange* splittableRange = nullptr;
    ChunkMemoryRange* suitableRange = nullptr;

    for (auto it = freeMemoryRanges.begin(); it != freeMemoryRanges.end(); ++it) {
        uint32_t sizeOfRange = it->endPos - it->startPos;

        if (sizeOfRange == requiredObjects) {
            suitableRange = &*it;
            eraseRangeIterator = it;
            break;
        }

        if (sizeOfRange > requiredObjects && sizeOfRange < bestSplittableRangeSize) {
            splittableRange = &*it;
            bestSplittableRangeSize = sizeOfRange;
            eraseRangeIterator = it;
        }
    }

    ChunkMemoryRange rangeToUse{};

    if (suitableRange != nullptr) {
        rangeToUse = *suitableRange;
        freeMemoryRanges.erase(eraseRangeIterator);
    }

    else if (splittableRange != nullptr) {
        rangeToUse = *splittableRange;
        freeMemoryRanges.erase(eraseRangeIterator);
        rangeToUse = splitUpAvailableMemory(freeMemoryRanges, &rangeToUse, requiredObjects);
    }

    else {
        const ChunkMemoryRange resizedRange = resizePool(poolType);
        rangeToUse = splitUpAvailableMemory(freeMemoryRanges, &resizedRange, requiredObjects);
    }

    initMemoryRangeInfo(rangeToUse, poolType, chunk.ID, offset, objectCount);
    occupiedRanges.push_back(rangeToUse);
    return rangeToUse;
}

void VertexPool::initMemoryRangeInfo(ChunkMemoryRange& rangeToUse, bool poolType, uint32_t chunkID,
    uint32_t offset, uint32_t objectCount) {
    if (poolType) {
        rangeToUse.offset = offset;
    }
    rangeToUse.chunkID = chunkID;
    rangeToUse.objectCount = objectCount;
    rangeToUse.savedToVBuffer = false;
}

// required slot must be power of 2
// returns a memory range == required space, and adds the rest from the split to the free ranges
ChunkMemoryRange VertexPool::splitUpAvailableMemory(std::vector<ChunkMemoryRange>& freeMemoryRanges,
    const ChunkMemoryRange* rangeToSplit, uint32_t requiredSpace) {

    ChunkMemoryRange newRange{0, rangeToSplit->startPos, rangeToSplit->startPos + requiredSpace};

    uint32_t remainingSpace = rangeToSplit->endPos - (rangeToSplit->startPos + requiredSpace);
    uint32_t nextRangeStartPos = rangeToSplit->startPos + requiredSpace;
    uint32_t currentPowerOf2 = requiredSpace;

    while (remainingSpace > 0) {
        freeMemoryRanges.push_back({
            0,
            nextRangeStartPos,
            nextRangeStartPos + currentPowerOf2});
        remainingSpace -= currentPowerOf2;
        nextRangeStartPos += currentPowerOf2;
        currentPowerOf2 *= 2;
    }

    return newRange;
}

ChunkMemoryRange VertexPool::resizePool(bool poolType) {
    if (poolType == 0) {
        const uint32_t currentSize = globalChunkVertices.size();
        const uint32_t goalSize = currentSize + CHUNK_VERTICES_SIZE;
        globalChunkVertices.resize(goalSize);
        return {0, currentSize, goalSize};
    }

    const uint32_t currentSize = globalChunkIndices.size();
    const uint32_t goalSize = currentSize + CHUNK_INDICES_SIZE;
    globalChunkIndices.resize(goalSize);
    return {0, currentSize, goalSize};
}
