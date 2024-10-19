#include "VertexPool.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

const static std::vector<uint32_t> powersOfTwo = {64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
static constexpr size_t CHUNK_VERTICES_SIZE = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 32; //must be a power of two


std::vector<ChunkVertex> globalChunkVertices(CHUNK_VERTICES_SIZE);
std::unordered_map<glm::vec3, ChunkMemoryRange> VertexPool::occupiedVertexRanges;
std::vector<ChunkMemoryRange> VertexPool::freeVertexRanges = {
    {0, CHUNK_VERTICES_SIZE, 0, 0, false, {0, 0, 0, 0, 0, 0}}
};
bool VertexPool::newUpdate;

void VertexPool::addToVertexPool(const ChunkVertex chunkVertices[MAX_QUADS], const uint32_t faceOffsets[6],
                                 const glm::vec3 &chunkID, uint32_t vertexCount) {
    const uint32_t requiredVertices = *std::lower_bound(powersOfTwo.begin(), powersOfTwo.end(), vertexCount);
    ChunkMemoryRange &vertexRangeToUse = getAvailableMemoryRange(occupiedVertexRanges, freeVertexRanges, chunkID,
                                                                 requiredVertices);
    vertexRangeToUse.indexCount = getAmountOfIndices(vertexCount);
    vertexRangeToUse.vertexCount = vertexCount;
    vertexRangeToUse.savedToVBuffer = false;
    std::copy_n(faceOffsets, 6, vertexRangeToUse.faceOffsets);
    std::copy_n(chunkVertices, vertexCount, globalChunkVertices.begin() + vertexRangeToUse.startPos);
    newUpdate = true;
}

ChunkMemoryRange &VertexPool::getAvailableMemoryRange(std::unordered_map<glm::vec3, ChunkMemoryRange> &occupiedRanges,
                                                      std::vector<ChunkMemoryRange> &freeMemoryRanges,
                                                      const glm::vec3 &chunkID, uint32_t requiredVertices) {
    // if the chunk has already been allocated memory, and it is enough space to save the new mesh, save it
    // otherwise, free up the chunk's occupied range and move on
    if (occupiedRanges.contains(chunkID)) {
        ChunkMemoryRange &occupiedRange = occupiedRanges.at(chunkID);

        if (occupiedRange.endPos - occupiedRange.startPos >= requiredVertices) {
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

        if (sizeOfRange == requiredVertices) {
            suitableRange = &*it;
            eraseRangeIterator = it;
            break;
        }

        if (sizeOfRange > requiredVertices && sizeOfRange < bestSplittableRangeSize) {
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
        rangeToUse = splitUpAvailableMemory(freeMemoryRanges, &rangeToUse, requiredVertices);
    } else {
        const ChunkMemoryRange resizedRange = resizePool();
        rangeToUse = splitUpAvailableMemory(freeMemoryRanges, &resizedRange, requiredVertices);
    }

    occupiedRanges[chunkID] = rangeToUse;
    return occupiedRanges[chunkID];
}

uint32_t VertexPool::getAmountOfIndices(uint32_t vertexCount) {
    return vertexCount + vertexCount / 2;
}

// required slot must be power of 2
// returns a memory range == required space, and adds the rest from the split to the free ranges
ChunkMemoryRange VertexPool::splitUpAvailableMemory(std::vector<ChunkMemoryRange> &freeMemoryRanges,
                                                    const ChunkMemoryRange *rangeToSplit, uint32_t requiredSpace) {
    const ChunkMemoryRange newRange{
        rangeToSplit->startPos, rangeToSplit->startPos + requiredSpace, 0, 0, false, {0, 0, 0, 0, 0, 0}
    };

    uint32_t remainingSpace = rangeToSplit->endPos - (rangeToSplit->startPos + requiredSpace);
    uint32_t nextRangeStartPos = rangeToSplit->startPos + requiredSpace;
    uint32_t currentPowerOf2 = requiredSpace;

    while (remainingSpace > 0) {
        freeMemoryRanges.push_back({
            nextRangeStartPos, nextRangeStartPos + currentPowerOf2, 0, 0, false, {0, 0, 0, 0, 0, 0}
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
ChunkMemoryRange VertexPool::resizePool() {
    const uint32_t currentSize = globalChunkVertices.size();
    const uint32_t goalSize = currentSize + CHUNK_VERTICES_SIZE;
    globalChunkVertices.resize(goalSize);
    return {currentSize, goalSize, 0, 0, false, {0, 0, 0, 0, 0, 0}};
}

std::unordered_map<glm::vec3, ChunkMemoryRange> &VertexPool::getOccupiedVertexRanges() {
    return occupiedVertexRanges;
}

uint32_t VertexPool::getIndexCount(ChunkMemoryRange memoryRange, int face) {
    switch (face) {
        case 0:
            return getAmountOfIndices(memoryRange.faceOffsets[1]);
        case 1:
            return getAmountOfIndices(memoryRange.faceOffsets[2] - memoryRange.faceOffsets[1]);
        case 2:
            return getAmountOfIndices(memoryRange.faceOffsets[3] - memoryRange.faceOffsets[2]);
        case 3:
            return getAmountOfIndices(memoryRange.faceOffsets[4] - memoryRange.faceOffsets[3]);
        case 4:
            return getAmountOfIndices(memoryRange.faceOffsets[5] - memoryRange.faceOffsets[4]);
        case 5:
            return getAmountOfIndices(memoryRange.vertexCount - memoryRange.faceOffsets[5]);
        default:
            throw std::runtime_error("error! incorrect face number entered into getIndexCount!");
    }
}
