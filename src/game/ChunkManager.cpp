#include "ChunkManager.h"

#include <iostream>
#include <stdexcept>
#include <glm/common.hpp>

#include "../rendering/misc/VertexPool.h"
#include "../util/GraphicsUtil.h"
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
    chunks[worldPos].octree = new InternalNode(glm::vec3(15.5f, 15.5f, 15.5f));
    chunks[worldPos].notMeshed = false;
    chunks[worldPos].ID = currentID++;
}

// credit/inspiration for this algorithm: Tantan's "Blazingly Fast Greedy Mesher - Voxel Engine Optimizations" video
void ChunkManager::meshChunk(ChunkVertex chunkVertices[MAX_QUADS], Chunk &chunk, const glm::vec3 &position,
                             uint32_t &vertexStart) {
    auto chunkCorner = glm::ivec3(position.x - HALF_CHUNK_SIZE, position.y - HALF_CHUNK_SIZE,
                                  position.z - HALF_CHUNK_SIZE);
    if (chunkCorner.x > 0) {
        chunkCorner.x += 1;
    }
    if (chunkCorner.y > 0) {
        chunkCorner.y += 1;
    }
    if (chunkCorner.z > 0) {
        chunkCorner.z += 1;
    }
    const InternalNode *chunkRoot = dynamic_cast<InternalNode *>(chunk.octree);
    uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED]{};
    uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED]{};
    Block chunkBlockInfo[CHUNK_SIZE_3];

    std::array<
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::array<uint32_t, CHUNK_SIZE> > >,
        6> planesPerBlock{};

    TimeManager::startTimer("setInternalBits");
    setInternalBits(chunkBitsPerAxis, chunkBlockInfo, chunkRoot);
    TimeManager::addTimeToProfiler("setInternalBits", TimeManager::finishTimer("setInternalBits"));

    TimeManager::startTimer("setExternalBits");
    setExternalBits(chunkBitsPerAxis, chunkCorner);
    TimeManager::addTimeToProfiler("setExternalBits", TimeManager::finishTimer("setExternalBits"));

    TimeManager::startTimer("cullBlockFaces");
    cullBlockFaces(culledFaces, chunkBitsPerAxis);
    TimeManager::addTimeToProfiler("cullBlockFaces", TimeManager::finishTimer("cullBlockFaces"));

    TimeManager::startTimer("generateBinaryPlanes");
    generateBinaryPlanes(planesPerBlock, culledFaces, chunkBlockInfo);
    TimeManager::addTimeToProfiler("generateBinaryPlanes", TimeManager::finishTimer("generateBinaryPlanes"));

    TimeManager::startTimer("meshAllBinaryPlanes");
    ChunkVertex vertexInfo = {glm::vec3(0), 0, 0, 0};
    meshAllBinaryPlanes(chunkVertices, planesPerBlock, vertexInfo, vertexStart, chunkCorner);
    TimeManager::addTimeToProfiler("meshAllBinaryPlanes", TimeManager::finishTimer("meshAllBinaryPlanes"));

    chunk.notMeshed = false;
}

void ChunkManager::setInternalBits(uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                   Block chunkBlockInfo[CHUNK_SIZE_3], const InternalNode *chunkRoot) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                const Block block = getBlock(chunkRoot, x, y, z);
                if (block.color[3] != 0) {
                    //x, z-y axis
                    chunkBitsPerAxis[0][z + 1][x + 1] |= 1ull << (y + 1);
                    //z, y-x axis
                    chunkBitsPerAxis[1][y + 1][z + 1] |= 1ull << (x + 1);
                    //x, y-z axis
                    chunkBitsPerAxis[2][y + 1][x + 1] |= 1ull << (z + 1);
                    //store block info for quick retrieval later
                    chunkBlockInfo[y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = block;
                }
            }
        }
    }
}

void ChunkManager::setAdjacentChunkBits(uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                        const InternalNode *chunkRoot, int xStart, int xEnd, int yStart, int yEnd,
                                        int zStart, int zEnd, int xOffset, int yOffset, int zOffset) {
    for (int z = zStart; z < zEnd; z++) {
        for (int y = yStart; y < yEnd; y++) {
            for (int x = xStart; x < xEnd; x++) {
                const Block block = getBlock(chunkRoot, x + xOffset, y + yOffset, z + zOffset);
                if (block.color[3] != 0) {
                    //x, z-y axis
                    chunkBitsPerAxis[0][z + 1][x + 1] |= 1ull << (y + 1);
                    //z, y-x axis
                    chunkBitsPerAxis[1][y + 1][z + 1] |= 1ull << (x + 1);
                    //x, y-z axis
                    chunkBitsPerAxis[2][y + 1][x + 1] |= 1ull << (z + 1);
                }
            }
        }
    }
}

void ChunkManager::setExternalBits(uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                   const glm::ivec3 &chunkCorner) {
    std::vector adjacentChunkPositions = {
        glm::ivec3(-1, 0, 0),
        glm::ivec3(CHUNK_SIZE, 0, 0),
        glm::ivec3(0, -1, 0),
        glm::ivec3(0, CHUNK_SIZE, 0),
        glm::ivec3(0, 0, -1),
        glm::ivec3(0, 0, CHUNK_SIZE),
    };

    std::vector loopEndNums = {
        glm::ivec3(0, CHUNK_SIZE, CHUNK_SIZE),
        glm::ivec3(CHUNK_SIZE + 1, CHUNK_SIZE, CHUNK_SIZE),
        glm::ivec3(CHUNK_SIZE, 0, CHUNK_SIZE),
        glm::ivec3(CHUNK_SIZE, CHUNK_SIZE + 1, CHUNK_SIZE),
        glm::ivec3(CHUNK_SIZE, CHUNK_SIZE, 0),
        glm::ivec3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE + 1),
    };

    for (int i = 0; i < 6; i++) {
        glm::ivec3 position = adjacentChunkPositions[i];
        const Chunk *chunk = getChunk(position + chunkCorner);
        if (chunk != nullptr) {
            glm::vec3 chunkCenter = Chunk::alignToChunkPos(position + chunkCorner);
            int xOffset = static_cast<int>(static_cast<float>(chunkCorner.x) - chunkCenter.x + HALF_CHUNK_SIZE);
            int yOffset = static_cast<int>(static_cast<float>(chunkCorner.y) - chunkCenter.y + HALF_CHUNK_SIZE);
            int zOffset = static_cast<int>(static_cast<float>(chunkCorner.z) - chunkCenter.z + HALF_CHUNK_SIZE);
            setAdjacentChunkBits(chunkBitsPerAxis, dynamic_cast<InternalNode *>(chunk->octree),
                                 position.x, loopEndNums[i].x,
                                 position.y, loopEndNums[i].y,
                                 position.z, loopEndNums[i].z,
                                 xOffset, yOffset, zOffset);
        }
    }
}

void ChunkManager::cullBlockFaces(uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                  uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED]) {
    for (int axis = 0; axis < 3; axis++) {
        for (int z = 0; z < CHUNK_SIZE_PADDED; z++) {
            for (int x = 0; x < CHUNK_SIZE_PADDED; x++) {
                const uint64_t column = chunkBitsPerAxis[axis][z][x];
                culledFaces[2 * axis + 0][z][x] = column & ~(column << 1);
                culledFaces[2 * axis + 1][z][x] = column & ~(column >> 1);
            }
        }
    }
}

void ChunkManager::generateBinaryPlanes(
    std::array<std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::array<uint32_t, CHUNK_SIZE> > >, 6> &
    planesPerBlock, uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED], Block chunkBlockInfo[CHUNK_SIZE_3]) {
    for (int axis = 0; axis < 6; axis++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                //skip over padding
                uint64_t column = culledFaces[axis][z + 1][x + 1];
                column >>= 1;
                column &= ~(1ull << CHUNK_SIZE);

                while (column != 0) {
                    const uint64_t y = __builtin_ctz(column);
                    column &= column - 1;

                    Block currentBlock{};
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
                    planesPerBlock[axis][blockKey][y][x] |= 1u << z;
                }
            }
        }
    }
}

void ChunkManager::meshAllBinaryPlanes(ChunkVertex chunkVertices[MAX_QUADS],
                                       std::array<std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::array<
                                           uint32_t, CHUNK_SIZE> > >, 6> &data,
                                       ChunkVertex &vertexInfo, uint32_t &vertexStart, const glm::ivec3 &chunkCorner) {
    for (int axis = 0; axis < 6; axis++) {
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::array<uint32_t, CHUNK_SIZE> > > axisInfo = data[axis];
        for (auto &[blockKey, axisPlanes]: axisInfo) {
            for (auto &[axisPos, plane]: axisPlanes) {
                vertexInfo.color[0] = blockKey & 255;
                vertexInfo.color[1] = (blockKey >> 8u) & 255;
                vertexInfo.color[2] = (blockKey >> 16u) & 255;
                greedyMeshBinaryPlane(chunkVertices, vertexStart, plane, chunkCorner, vertexInfo, axis, axisPos);
            }
        }
    }
}

void ChunkManager::greedyMeshBinaryPlane(ChunkVertex chunkVertices[MAX_QUADS], uint32_t &vertexStart,
                                         std::array<uint32_t, CHUNK_SIZE> &plane,
                                         const glm::ivec3 &chunkCorner, ChunkVertex &vertexInfo, uint32_t axis,
                                         uint32_t axisPos) {
    for (int row = 0; row < CHUNK_SIZE; row++) {
        uint32_t y = 0;
        while (y < CHUNK_SIZE) {
            y += __builtin_ctz(plane[row] >> y);
            if (y >= CHUNK_SIZE) {
                continue;
            }
            uint32_t height = __builtin_ctz(~(plane[row] >> y));
            uint32_t heightAsMask;
            if (height >= 32) {
                heightAsMask = std::numeric_limits<uint32_t>::max();
            } else {
                heightAsMask = (1u << height) - 1;
            }
            uint32_t mask = heightAsMask << y;
            uint32_t width = 1;
            while (row + width < CHUNK_SIZE) {
                uint32_t nextRowHeight = (plane[row + width] >> y) & heightAsMask;
                if (nextRowHeight != heightAsMask) {
                    break;
                }
                plane[row + width] = plane[row + width] & ~mask;
                width += 1;
            }
            insertGreedyQuad(chunkVertices, vertexStart, vertexInfo, chunkCorner, row, y, width, height, axis, axisPos);
            y += height;
        }
    }
}

void ChunkManager::generateBlockMesh(Chunk &chunk, const glm::ivec3 &chunkCorner, Block block, int x, int y, int z) {
    const auto adjustedPosition = glm::vec3(x + chunkCorner.x, y + chunkCorner.y, z + chunkCorner.z);

    if (!hasBlock(adjustedPosition + glm::vec3(0, 1, 0))) {
        insertBlockVertices(chunk.vertices, adjustedPosition, block.color, 0);
    }
    if (!hasBlock(adjustedPosition + glm::vec3(0, -1, 0))) {
        insertBlockVertices(chunk.vertices, adjustedPosition, block.color, 1);
    }
    if (!hasBlock(adjustedPosition + glm::vec3(0, 0, 1))) {
        insertBlockVertices(chunk.vertices, adjustedPosition, block.color, 2);
    }
    if (!hasBlock(adjustedPosition + glm::vec3(0, 0, -1))) {
        insertBlockVertices(chunk.vertices, adjustedPosition, block.color, 3);
    }
    if (!hasBlock(adjustedPosition + glm::vec3(-1, 0, 0))) {
        insertBlockVertices(chunk.vertices, adjustedPosition, block.color, 4);
    }
    if (!hasBlock(adjustedPosition + glm::vec3(1, 0, 0))) {
        insertBlockVertices(chunk.vertices, adjustedPosition, block.color, 5);
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
    newBlock->color[3] = block.color[3];
    chunk->notMeshed = true;
}

Block *ChunkManager::createPathToBlock(const Chunk *chunk, const glm::vec3 &position) {
    auto *currentNode = dynamic_cast<InternalNode *>(chunk->octree);
    OctreeNode *newBlockNode = nullptr;
    int depth = 0;

    while (depth < MAX_DEPTH) {
        const int octantIndex = Chunk::getOctantIndex(position, currentNode->position);

        if (depth < MAX_DEPTH - 1 && currentNode->children[octantIndex] != nullptr) {
            currentNode = dynamic_cast<InternalNode *>(currentNode->children[octantIndex]);
            depth++;
            continue;
        }

        if (currentNode->children[octantIndex] != nullptr) {
            newBlockNode = currentNode->children[octantIndex];
            break;
        }

        glm::vec3 childPos = currentNode->position;
        Chunk::addOctantOffset(childPos, octantIndex, depth);

        if (depth < MAX_DEPTH - 1) {
            currentNode->children[octantIndex] = new InternalNode(childPos);
            currentNode = dynamic_cast<InternalNode *>(currentNode->children[octantIndex]);
        } else {
            currentNode->children[octantIndex] = new OctreeNode();
            newBlockNode = currentNode->children[octantIndex];
        }

        depth++;
    }

    if (newBlockNode == nullptr) {
        throw std::runtime_error("failed to add block!");
    }

    return &newBlockNode->block;
}

Block ChunkManager::getBlock(const InternalNode *treeRoot, int x, int y, int z) {
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
        if (treeRoot->children[childIndex] == nullptr) {
            return {};
        }
        if (depth == MAX_DEPTH - 1) {
            return treeRoot->children[childIndex]->block;
        }
        treeRoot = dynamic_cast<InternalNode *>(treeRoot->children[childIndex]);
        depth++;
    }
    return treeRoot->block;
}

Block ChunkManager::getBlock(const int x, const int y, const int z) {
    Chunk *chunk = getChunk(glm::vec3(x, y, z));
    if (chunk == nullptr) {
        return {};
    }
    const InternalNode *chunkRoot = dynamic_cast<InternalNode *>(chunk->octree);
    glm::vec3 chunkCenter = Chunk::alignToChunkPos(glm::vec3(x, y, z));
    return getBlock(chunkRoot,
                    x - chunkCenter.x + HALF_CHUNK_SIZE,
                    y - chunkCenter.y + HALF_CHUNK_SIZE,
                    z - chunkCenter.z + HALF_CHUNK_SIZE);
}

bool ChunkManager::hasBlock(const glm::vec3 &worldPos) {
    const OctreeNode *blockTree = findOctreeNode(worldPos);
    return blockTree != nullptr;
}

void ChunkManager::removeBlock(const glm::vec3 &worldPos) {
    //todo testing
    OctreeNode *blockTree = findOctreeNode(worldPos);

    if (blockTree == nullptr) {
        throw std::runtime_error("error removing block!");
    }

    blockTree->block = {};

    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);

    if (Chunk *chunk = getChunk(chunkCenter); chunk != nullptr) {
        chunk->notMeshed = true;
    }
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

OctreeNode *ChunkManager::findOctreeNode(const glm::vec3 &worldPos) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);
    const Chunk *chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        return nullptr;
    }

    auto *currentNode = dynamic_cast<InternalNode *>(chunk->octree);
    glm::ivec3 relativePosition = worldPos - chunkCenter + glm::vec3(HALF_CHUNK_SIZE);

    int depth = 0;
    while (depth < MAX_DEPTH) {
        int childIndex = 0;
        if (static_cast<float>(relativePosition.x) >= currentNode->position.x) childIndex |= 1;
        if (static_cast<float>(relativePosition.y) >= currentNode->position.y) childIndex |= 2;
        if (static_cast<float>(relativePosition.z) >= currentNode->position.z) childIndex |= 4;

        if (currentNode->children[childIndex] == nullptr) {
            return nullptr;
        }

        if (depth == MAX_DEPTH - 1) {
            return currentNode->children[childIndex];
        }

        currentNode = dynamic_cast<InternalNode *>(currentNode->children[childIndex]);
        depth++;
    }

    return currentNode;
}

void ChunkManager::meshAllChunks() {
    ChunkVertex chunkVertices[MAX_QUADS]{};
    chunkVertices[0] = {};
    for (auto &[position, chunk]: chunks) {
        if (chunk.notMeshed) {
            uint32_t vertexCount = 0;
            TimeManager::startTimer("meshChunk");
            meshChunk(chunkVertices, chunk, position, vertexCount); //todo don't store vertices in chunks
            TimeManager::addTimeToProfiler("meshChunk", TimeManager::finishTimer("meshChunk"));

            TimeManager::startTimer("addToVertexPool");
            if (vertexCount > 0) {
                VertexPool::addToVertexPool(chunkVertices, vertexCount, chunk.ID);
            }
            TimeManager::addTimeToProfiler("addToVertexPool", TimeManager::finishTimer("addToVertexPool"));
        }
    }
}

uint32_t ChunkManager::chunkCount() const {
    return chunks.size();
}
