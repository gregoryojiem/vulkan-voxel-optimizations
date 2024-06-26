#include "VertexPool.h"

#include <iostream>

const static std::vector<uint32_t> powersOfTwo = {64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
static constexpr size_t CHUNK_VERTICES_SIZE = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 32; //must be a power of two

std::vector<ChunkVertex> globalChunkVertices(CHUNK_VERTICES_SIZE);
std::unordered_map<uint32_t, ChunkMemoryRange> VertexPool::occupiedVertexRanges;
std::vector<ChunkMemoryRange> VertexPool::freeVertexRanges = {{0, CHUNK_VERTICES_SIZE, 0, 0, false}};
bool VertexPool::newUpdate;

void VertexPool::addToVertexPool(const ChunkVertex chunkVertices[MAX_QUADS], uint32_t vertexCount, uint32_t chunkID) {
    const ChunkMemoryRange vertexRangeToUse = getAvailableMemoryRange(occupiedVertexRanges, freeVertexRanges, chunkID,
                                                                      vertexCount);
    std::copy_n(chunkVertices, vertexCount, globalChunkVertices.begin() + vertexRangeToUse.startPos);
    newUpdate = true;
}

std::unordered_map<uint32_t, ChunkMemoryRange> &VertexPool::getOccupiedVertexRanges() {
    return occupiedVertexRanges;
}

ChunkMemoryRange VertexPool::getAvailableMemoryRange(std::unordered_map<uint32_t, ChunkMemoryRange> &occupiedRanges,
                                                     std::vector<ChunkMemoryRange> &freeMemoryRanges, uint32_t chunkID,
                                                     const uint16_t objectCount) {
    const auto requiredObjects = *std::lower_bound(powersOfTwo.begin(), powersOfTwo.end(), objectCount);
    // if the chunk has already been allocated memory, and it is enough space to save the new mesh, save it
    // otherwise, free up the chunk's occupied range and move on
    if (occupiedRanges.contains(chunkID)) {
        ChunkMemoryRange &occupiedRange = occupiedRanges.at(chunkID);

        if (occupiedRange.endPos - occupiedRange.startPos >= requiredObjects) {
            initMemoryRangeInfo(occupiedRange, objectCount);
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
        const ChunkMemoryRange resizedRange = resizePool();
        rangeToUse = splitUpAvailableMemory(freeMemoryRanges, &resizedRange, requiredObjects);
    }

    initMemoryRangeInfo(rangeToUse, objectCount);
    occupiedRanges[chunkID] = rangeToUse;
    return rangeToUse;
}

void VertexPool::initMemoryRangeInfo(ChunkMemoryRange &rangeToUse, uint32_t objectCount) {
    rangeToUse.indexCount = objectCount + objectCount / 2;
    rangeToUse.vertexCount = objectCount;
    rangeToUse.savedToVBuffer = false;
}

// required slot must be power of 2
// returns a memory range == required space, and adds the rest from the split to the free ranges
ChunkMemoryRange VertexPool::splitUpAvailableMemory(std::vector<ChunkMemoryRange> &freeMemoryRanges,
                                                    const ChunkMemoryRange *rangeToSplit, uint32_t requiredSpace) {
    const ChunkMemoryRange newRange{rangeToSplit->startPos, rangeToSplit->startPos + requiredSpace, 0, 0, false};

    uint32_t remainingSpace = rangeToSplit->endPos - (rangeToSplit->startPos + requiredSpace);
    uint32_t nextRangeStartPos = rangeToSplit->startPos + requiredSpace;
    uint32_t currentPowerOf2 = requiredSpace;

    while (remainingSpace > 0) {
        freeMemoryRanges.push_back({nextRangeStartPos, nextRangeStartPos + currentPowerOf2, 0, 0, false});
        remainingSpace -= currentPowerOf2;
        nextRangeStartPos += currentPowerOf2;
        currentPowerOf2 *= 2;
    }

    return newRange;
}

//TODO: possible optimizations
//this function takes up around ~77.8% of the execution time in the VertexPool class, and there is room to explore
//different allocation sizes, but it's low priority compared to reducing memory usage in other areas
ChunkMemoryRange VertexPool::resizePool() {
    const uint32_t currentSize = globalChunkVertices.size();
    const uint32_t goalSize = currentSize + CHUNK_VERTICES_SIZE;
    globalChunkVertices.resize(goalSize);
    return {currentSize, goalSize, 0, 0, false};
}
