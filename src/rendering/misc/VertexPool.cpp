#include "VertexPool.h"

#include <iostream>

std::vector<ChunkVertex> globalChunkVertices(CHUNK_VERTICES_SIZE);
std::vector<uint32_t> globalChunkIndices(CHUNK_INDICES_SIZE);

std::unordered_map<uint32_t, ChunkMemoryRange> VertexPool::occupiedVertexRanges;
std::unordered_map<uint32_t, ChunkMemoryRange> VertexPool::occupiedIndexRanges;
std::vector<ChunkMemoryRange> VertexPool::freeVertexRanges = {{0, CHUNK_VERTICES_SIZE, CHUNK_VERTICES_SIZE}};
std::vector<ChunkMemoryRange> VertexPool::freeIndexRanges = {{0, CHUNK_INDICES_SIZE, CHUNK_INDICES_SIZE}};
bool VertexPool::newUpdate;

void VertexPool::addToVertexPool(const std::vector<ChunkVertex> &vertices, const std::vector<uint32_t> &indices,
                                 uint32_t chunkID) {
    ChunkMemoryRange vertexRangeToUse = getAvailableMemoryRange(occupiedVertexRanges, freeVertexRanges, chunkID,
                                                                0, vertices.size(), false);

    ChunkMemoryRange indexRangeToUse = getAvailableMemoryRange(occupiedIndexRanges, freeIndexRanges, chunkID,
                                                               vertexRangeToUse.startPos, indices.size(), true);

    std::copy(vertices.begin(), vertices.end(), globalChunkVertices.begin() + vertexRangeToUse.startPos);
    std::copy(indices.begin(), indices.end(), globalChunkIndices.begin() + indexRangeToUse.startPos);

    newUpdate = true;
}

std::unordered_map<uint32_t, ChunkMemoryRange> &VertexPool::getOccupiedVertexRanges() {
    return occupiedVertexRanges;
}

std::unordered_map<uint32_t, ChunkMemoryRange> &VertexPool::getOccupiedIndexRanges() {
    return occupiedIndexRanges;
}

ChunkMemoryRange VertexPool::getAvailableMemoryRange(std::unordered_map<uint32_t, ChunkMemoryRange> &occupiedRanges,
                                                     std::vector<ChunkMemoryRange> &freeMemoryRanges, uint32_t chunkID,
                                                     uint32_t offset, const uint16_t objectCount,
                                                     const bool poolType) {
    const auto requiredObjects = *std::lower_bound(powersOfTwo.begin(), powersOfTwo.end(), objectCount);
    // if the chunk has already been allocated memory, and it is enough space to save the new mesh, save it
    // otherwise, free up the chunk's occupied range and move on
    if (occupiedRanges.contains(chunkID)) {
        ChunkMemoryRange &occupiedRange = occupiedRanges.at(chunkID);

        if (occupiedRange.endPos - occupiedRange.startPos >= requiredObjects) {
            initMemoryRangeInfo(occupiedRange, poolType, offset, objectCount);
            return occupiedRange;
        }

        freeMemoryRanges.push_back(occupiedRange);
        occupiedRanges.erase(chunkID);
    }

    // look for a suitable existing range with the correct size
    // if no existing range can be found, find an available one that can be split up
    uint32_t bestSplittableRangeSize = UINT32_MAX;
    auto eraseRangeIterator = freeMemoryRanges.end();

    ChunkMemoryRange *splittableRange = nullptr;
    ChunkMemoryRange *suitableRange = nullptr;
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
    } else if (splittableRange != nullptr) {
        rangeToUse = *splittableRange;
        freeMemoryRanges.erase(eraseRangeIterator);
        rangeToUse = splitUpAvailableMemory(freeMemoryRanges, &rangeToUse, requiredObjects);
    } else {
        const ChunkMemoryRange resizedRange = resizePool(poolType);
        rangeToUse = splitUpAvailableMemory(freeMemoryRanges, &resizedRange, requiredObjects);
    }

    initMemoryRangeInfo(rangeToUse, poolType, offset, objectCount);
    occupiedRanges[chunkID] = rangeToUse;
    return rangeToUse;
}

void VertexPool::initMemoryRangeInfo(ChunkMemoryRange &rangeToUse, bool poolType, uint32_t offset,
                                     uint32_t objectCount) {
    if (poolType) {
        rangeToUse.offset = offset;
    }
    rangeToUse.objectCount = objectCount;
    rangeToUse.savedToVBuffer = false;
}

// required slot must be power of 2
// returns a memory range == required space, and adds the rest from the split to the free ranges
ChunkMemoryRange VertexPool::splitUpAvailableMemory(std::vector<ChunkMemoryRange> &freeMemoryRanges,
                                                    const ChunkMemoryRange *rangeToSplit, uint32_t requiredSpace) {
    const ChunkMemoryRange newRange{rangeToSplit->startPos, rangeToSplit->startPos + requiredSpace};

    uint32_t remainingSpace = rangeToSplit->endPos - (rangeToSplit->startPos + requiredSpace);
    uint32_t nextRangeStartPos = rangeToSplit->startPos + requiredSpace;
    uint32_t currentPowerOf2 = requiredSpace;

    while (remainingSpace > 0) {
        freeMemoryRanges.push_back({
            nextRangeStartPos,
            nextRangeStartPos + currentPowerOf2
        });
        remainingSpace -= currentPowerOf2;
        nextRangeStartPos += currentPowerOf2;
        currentPowerOf2 *= 2;
    }

    return newRange;
}

//TODO: possible optimizations
//this function takes up around ~77.8% of the execution time in the VertexPool class, and there is room to explore
//different allocation sizes, but it's low priority compared to reducing memory usage in other areas
ChunkMemoryRange VertexPool::resizePool(bool poolType) {
    if (poolType == 0) {
        const uint32_t currentSize = globalChunkVertices.size();
        const uint32_t goalSize = currentSize + CHUNK_VERTICES_SIZE;
        globalChunkVertices.resize(goalSize);
        return {currentSize, goalSize};
    }

    const uint32_t currentSize = globalChunkIndices.size();
    const uint32_t goalSize = currentSize + CHUNK_INDICES_SIZE;
    globalChunkIndices.resize(goalSize);
    return {currentSize, goalSize};
}
