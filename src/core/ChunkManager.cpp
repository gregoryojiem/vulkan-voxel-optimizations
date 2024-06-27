#include "ChunkManager.h"

#include <iostream>
#include <stack>
#include <stdexcept>
#include <glm/common.hpp>

#include "../rendering/scene/VertexPool.h"
#include "../util/VertexUtil.h"
#include "../util/TimeManager.h"

uint32_t ChunkManager::currentID = 1;

Chunk *ChunkManager::getChunk(const glm::vec3 &worldPos) {
    auto it = chunks.find(Chunk::alignToChunkPos(worldPos));
    if (it != chunks.end()) {
        return &it->second;
    }
    return nullptr;
}

void ChunkManager::createChunk(const glm::vec3 &worldPos) {
    glm::vec3 normalizedChunkPos = (worldPos - CHUNK_SHIFT) / glm::vec3(static_cast<float>(CHUNK_SIZE));

    if (std::floor(normalizedChunkPos.x) != normalizedChunkPos.x ||
        std::floor(normalizedChunkPos.y) != normalizedChunkPos.y ||
        std::floor(normalizedChunkPos.z) != normalizedChunkPos.z) {
        throw std::runtime_error("chunk creation error: location invalid!");
    }

    if (chunks.contains(worldPos)) {
        throw std::runtime_error("chunk creation error: chunk already exists!");
    }

    chunks[worldPos] = Chunk();
    chunks[worldPos].octree = new OctreeNode(glm::vec3(15.5f, 15.5f, 15.5f));
    chunks[worldPos].notMeshed = false;
    chunks[worldPos].ID = currentID++;
}

// credit/inspiration for this algorithm: Tantan's "Blazingly Fast Greedy Mesher - Voxel Engine Optimizations" video
void ChunkManager::meshChunk(ChunkVertex chunkVertices[MAX_QUADS], uint32_t &vertexStart, const OctreeNode *chunkRoot,
                             const glm::vec3 &chunkCorner) {
    Block chunkBlocks[CHUNK_SIZE_3];
    uint64_t solidBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED]{};
    uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED]{};
    std::unordered_map<uint32_t, uint32_t[CHUNK_SIZE][CHUNK_SIZE]> blockPlanes[6];
    bool initializedPlanes[CHUNK_SIZE]{};
    ChunkVertex vertexInfo = {glm::vec3(0), 0, 0, 0};

    TimeManager::startTimer("setInternalBits");
    setInternalBits(solidBitsPerAxis, chunkBlocks, chunkRoot);
    TimeManager::addTimeToProfiler("setInternalBits", TimeManager::finishTimer("setInternalBits"));

    TimeManager::startTimer("setExternalBits");
    setExternalBits(solidBitsPerAxis, chunkCorner);
    TimeManager::addTimeToProfiler("setExternalBits", TimeManager::finishTimer("setExternalBits"));

    TimeManager::startTimer("cullBlockFaces");
    cullBlockFaces(culledFaces, solidBitsPerAxis);
    TimeManager::addTimeToProfiler("cullBlockFaces", TimeManager::finishTimer("cullBlockFaces"));

    TimeManager::startTimer("generateBinaryPlanes");
    generateBinaryPlanes(blockPlanes, initializedPlanes, culledFaces, chunkBlocks);
    TimeManager::addTimeToProfiler("generateBinaryPlanes", TimeManager::finishTimer("generateBinaryPlanes"));

    TimeManager::startTimer("meshAllBinaryPlanes");
    meshAllBinaryPlanes(chunkVertices, blockPlanes, initializedPlanes, vertexInfo, vertexStart, chunkCorner);
    TimeManager::addTimeToProfiler("meshAllBinaryPlanes", TimeManager::finishTimer("meshAllBinaryPlanes"));
}

void ChunkManager::setInternalBits(uint64_t solidBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                   Block chunkBlocks[CHUNK_SIZE_3], const OctreeNode *root) {
    if (root->isLeafNode) {
        for (const auto block: root->blocks) {
            if (!block.initialized) {
                continue;
            }
            //x, z-y axis
            solidBitsPerAxis[0][block.position[2] + 1][block.position[0] + 1] |= 1ull << (block.position[1] + 1);
            //z, y-x axis
            solidBitsPerAxis[1][block.position[1] + 1][block.position[2] + 1] |= 1ull << (block.position[0] + 1);
            //x, y-z axis
            solidBitsPerAxis[2][block.position[1] + 1][block.position[0] + 1] |= 1ull << (block.position[2] + 1);
            //store block info for quick retrieval later
            chunkBlocks[block.position[1] * CHUNK_SIZE_2 + block.position[2] * CHUNK_SIZE + block.position[0]] = block;
        }
        return;
    }

    for (const OctreeNode *const &childNode: root->children) {
        if (childNode == nullptr) {
            continue;
        }
        setInternalBits(solidBitsPerAxis, chunkBlocks, childNode);
    }
}

AdjacentChunkBounds::AdjacentChunkBounds(const int xOffset, const int yOffset, const int zOffset) {
    xMin = yMin = zMin = 0;
    xMax = yMax = zMax = CHUNK_SIZE;
    this->xOffset = static_cast<int8_t>(xOffset);
    this->yOffset = static_cast<int8_t>(yOffset);
    this->zOffset = static_cast<int8_t>(zOffset);

    auto adjust = [](int8_t &offset, int8_t &min, int8_t &max) {
        max = static_cast<int8_t>(offset < 0 ? 1 : max);
        min = static_cast<int8_t>(offset > 0 ? 31 : min);
        switch (offset) {
            case -1:
                offset = 33;
            break;
            case 1:
                offset = -31;
            break;
            default:
                offset = 1;
        }
    };

    adjust(this->xOffset, xMin, xMax);
    adjust(this->yOffset, yMin, yMax);
    adjust(this->zOffset, zMin, zMax);
}

void ChunkManager::setAdjacentChunkBits(uint64_t solidBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                        const OctreeNode *root, const AdjacentChunkBounds &loopInfo) {
    for (int8_t z = loopInfo.zMin; z < loopInfo.zMax; z++) {
        for (int8_t y = loopInfo.yMin; y < loopInfo.yMax; y++) {
            for (int8_t x = loopInfo.xMin; x < loopInfo.xMax; x++) {
                if (const Block block = getBlock(root, x, y, z); block.initialized) {
                    //x, z-y axis
                    solidBitsPerAxis[0][z + loopInfo.zOffset][x + loopInfo.xOffset] |= 1ull << (y + loopInfo.yOffset);
                    //z, y-x axis
                    solidBitsPerAxis[1][y + loopInfo.yOffset][z + loopInfo.zOffset] |= 1ull << (x + loopInfo.xOffset);
                    //x, y-z axis
                    solidBitsPerAxis[2][y + loopInfo.yOffset][x + loopInfo.xOffset] |= 1ull << (z + loopInfo.zOffset);
                }
            }
        }
    }
}

void ChunkManager::setExternalBits(uint64_t solidBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                   const glm::ivec3 &chunkCorner) {
    static const std::vector adjacentChunkPositions = {
        glm::ivec3(0, -1, 0),
        glm::ivec3(0, CHUNK_SIZE, 0),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(CHUNK_SIZE, 0, 0),
        glm::ivec3(0, 0, -1),
        glm::ivec3(0, 0, CHUNK_SIZE),
    };

    glm::vec3 baseChunkCenter = glm::vec3(chunkCorner + HALF_CHUNK_SIZE);
    for (int axis = 0; axis < 6; axis++) {
        glm::ivec3 position = adjacentChunkPositions[axis];
        if (const Chunk *chunk = getChunk(position + chunkCorner); chunk != nullptr) {
            const glm::vec3 adjacentChunkCenter = Chunk::alignToChunkPos(position + chunkCorner);
            const int xOffset = std::clamp(static_cast<int>(baseChunkCenter.x - adjacentChunkCenter.x), -1, 1);
            const int yOffset = std::clamp(static_cast<int>(baseChunkCenter.y - adjacentChunkCenter.y), -1, 1);
            const int zOffset = std::clamp(static_cast<int>(baseChunkCenter.z - adjacentChunkCenter.z), -1, 1);
            setAdjacentChunkBits(solidBitsPerAxis, chunk->octree, AdjacentChunkBounds(xOffset, yOffset, zOffset));
        }
    }
}

void ChunkManager::cullBlockFaces(uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                  const uint64_t solidBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED]) {
    for (uint32_t axis = 0; axis < 3; axis++) {
        for (uint32_t z = 0; z < CHUNK_SIZE_PADDED; z++) {
            for (uint32_t x = 0; x < CHUNK_SIZE_PADDED; x++) {
                const uint64_t column = solidBitsPerAxis[axis][z][x];
                culledFaces[2 * axis + 0][z][x] = column & ~(column << 1);
                culledFaces[2 * axis + 1][z][x] = column & ~(column >> 1);
            }
        }
    }
}

void ChunkManager::generateBinaryPlanes(std::unordered_map<uint32_t, uint32_t[CHUNK_SIZE][CHUNK_SIZE]> blockPlanes[6],
                                        bool initializedPlanes[CHUNK_SIZE],
                                        const uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                        const Block chunkBlockInfo[CHUNK_SIZE_3]) {
    for (uint32_t axis = 0; axis < 6; axis++) {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++) {
            for (uint32_t x = 0; x < CHUNK_SIZE; x++) {
                uint64_t column = culledFaces[axis][z + 1][x + 1];
                column >>= 1; //skip over padding
                column &= ~(1ull << CHUNK_SIZE);

                while (column != 0) {
                    const uint64_t y = __builtin_ctz(column);
                    column &= column - 1;

                    Block currentBlock;
                    switch (axis) {
                        case 0:
                        case 1:
                            currentBlock = chunkBlockInfo[y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x];
                            break;
                        case 2:
                        case 3:
                            currentBlock = chunkBlockInfo[z * CHUNK_SIZE * CHUNK_SIZE + x * CHUNK_SIZE + y];
                            break;
                        default:
                            currentBlock = chunkBlockInfo[z * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + x];
                            break;
                    }

                    uint32_t blockKey = 0;
                    blockKey |= currentBlock.color[0];
                    blockKey |= currentBlock.color[1] << 8u;
                    blockKey |= currentBlock.color[2] << 16u;
                    blockPlanes[axis][blockKey][y][x] |= 1u << z;
                    initializedPlanes[y] = true;
                }
            }
        }
    }
}

void ChunkManager::meshAllBinaryPlanes(ChunkVertex chunkVertices[MAX_QUADS],
                                       std::unordered_map<uint32_t, uint32_t[CHUNK_SIZE][CHUNK_SIZE]> blockPlanes[6],
                                       const bool initializedPlanes[CHUNK_SIZE],
                                       ChunkVertex &vertexInfo, uint32_t &vertexStart, const glm::ivec3 &chunkCorner) {
    for (uint32_t axis = 0; axis < 6; axis++) {
        for (auto &[blockKey, axisPlanes]: blockPlanes[axis]) {
            for (uint32_t i = 0; i < CHUNK_SIZE; i++) {
                if (initializedPlanes[i]) {
                    vertexInfo.color[0] = blockKey & 255;
                    vertexInfo.color[1] = (blockKey >> 8) & 255;
                    vertexInfo.color[2] = (blockKey >> 16) & 255;
                    greedyMeshBinaryPlane(chunkVertices, vertexStart, axisPlanes[i], chunkCorner, vertexInfo, axis, i);
                }
            }
        }
    }
}

void ChunkManager::greedyMeshBinaryPlane(ChunkVertex chunkVertices[MAX_QUADS], uint32_t &vertexStart,
                                         uint32_t plane[CHUNK_SIZE], const glm::ivec3 &chunkCorner,
                                         const ChunkVertex &vertexInfo, uint32_t axis, uint32_t axisPos) {
    for (uint32_t x = 0; x < CHUNK_SIZE; x++) {
        uint32_t y = 0;
        while (y < CHUNK_SIZE) {
            y += __builtin_ctz(plane[x] >> y);
            if (y >= CHUNK_SIZE) {
                continue;
            }

            uint32_t height;
            if (plane[x] >> y == UINT32_MAX) {
                height = 32;
            } else {
                height = __builtin_ctz(~(plane[x] >> y));
            }

            uint32_t heightAsMask;
            if (height >= 32) {
                heightAsMask = std::numeric_limits<uint32_t>::max();
            } else {
                heightAsMask = (1u << height) - 1;
            }

            const uint32_t mask = heightAsMask << y;
            uint32_t width = 1;
            while (x + width < CHUNK_SIZE) {
                if ((plane[x + width] >> y & heightAsMask) != heightAsMask) {
                    break;
                }
                plane[x + width] = plane[x + width] & ~mask;
                width += 1;
            }
            //todo axis won't be needed once normals are set up
            insertGreedyQuad(chunkVertices, vertexStart, vertexInfo, chunkCorner, x, y, width, height, axis, axisPos);
            y += height;
        }
    }
}

void ChunkManager::addBlock(const glm::vec3 &position, const Block &block) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(position);
    Chunk *chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        createChunk(chunkCenter);
        chunk = getChunk(chunkCenter);
    }

    glm::ivec3 relativePosition = position - chunkCenter + glm::vec3(HALF_CHUNK_SIZE);
    Block *newBlock = createPathToBlock(chunk, relativePosition);
    newBlock->position[0] = static_cast<int8_t>(relativePosition.x);
    newBlock->position[1] = static_cast<int8_t>(relativePosition.y);
    newBlock->position[2] = static_cast<int8_t>(relativePosition.z);
    newBlock->color[0] = block.color[0];
    newBlock->color[1] = block.color[1];
    newBlock->color[2] = block.color[2];
    newBlock->initialized = true;
    chunk->notMeshed = true;
}

Block *ChunkManager::createPathToBlock(const Chunk *chunk, const glm::vec3 &position) {
    auto *currentNode = chunk->octree;
    int depth = 0;
    int octantIndex;

    while (depth < MAX_DEPTH - 1) {
        octantIndex = Chunk::getOctantIndex(position, currentNode->position);
        if (currentNode->children[octantIndex] != nullptr) {
            currentNode = currentNode->children[octantIndex];
            depth++;
            continue;
        }

        glm::vec3 childPos = currentNode->position;
        Chunk::addOctantOffset(childPos, octantIndex, depth);
        currentNode->children[octantIndex] = new OctreeNode(childPos);
        currentNode = currentNode->children[octantIndex];
        depth++;
    }
    currentNode->isLeafNode = true;
    octantIndex = Chunk::getOctantIndex(position, currentNode->position);
    return &currentNode->blocks[octantIndex];
}

Block ChunkManager::getBlock(const OctreeNode *treeRoot, int x, int y, int z) {
    int depth = 0;
    while (depth < MAX_DEPTH) {
        int childIndex = 0;
        if (x >= treeRoot->position.x) {
            childIndex |= 1;
        }
        if (y >= treeRoot->position.y) {
            childIndex |= 2;
        }
        if (z >= treeRoot->position.z) {
            childIndex |= 4;
        }
        if (depth == MAX_DEPTH - 1) {
            return treeRoot->blocks[childIndex];
        }
        if (treeRoot->children[childIndex] == nullptr) {
            return {};
        }
        treeRoot = treeRoot->children[childIndex];
        depth++;
    }
    return {};
}

void ChunkManager::fillChunk(const glm::vec3 &worldPos, const glm::vec3 &position, const Block &block) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);

    if (const Chunk *chunk = getChunk(chunkCenter); chunk == nullptr) {
        createChunk(chunkCenter);
    }

    const glm::vec3 chunkCorner = chunkCenter - CHUNK_SHIFT;
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                Block newBlock = block;
                newBlock.position[0] = static_cast<int8_t>(chunkCorner.x + static_cast<float>(i));
                newBlock.position[1] = static_cast<int8_t>(chunkCorner.y + static_cast<float>(j));
                newBlock.position[2] = static_cast<int8_t>(chunkCorner.z + static_cast<float>(k));
                addBlock(position, newBlock);
            }
        }
    }
}

int ChunkManager::meshAllChunks() {
    ChunkVertex chunkVertices[MAX_QUADS]{};
    chunkVertices[0] = {};
    int totalVertexCount = 0;
    for (auto &[position, chunk]: chunks) {
        if (chunk.notMeshed) {
            uint32_t vertexCount = 0;
            TimeManager::startTimer("meshChunk");
            meshChunk(chunkVertices, vertexCount, chunk.octree, Chunk::calculateCorner(position));
            chunk.notMeshed = false;
            TimeManager::addTimeToProfiler("meshChunk", TimeManager::finishTimer("meshChunk"));

            TimeManager::startTimer("addToVertexPool");
            if (vertexCount > 0) {
                VertexPool::addToVertexPool(chunkVertices, vertexCount, chunk.ID);
            }
            TimeManager::addTimeToProfiler("addToVertexPool", TimeManager::finishTimer("addToVertexPool"));
            totalVertexCount += vertexCount;
        }
    }
    const int triangleCount = totalVertexCount / 2;
    return triangleCount;
}

uint32_t ChunkManager::chunkCount() const {
    return chunks.size();
}
