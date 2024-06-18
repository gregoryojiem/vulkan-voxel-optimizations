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
    chunks[worldPos].octree = new InternalNode(worldPos);
    chunks[worldPos].geometryModified = false;
    chunks[worldPos].ID = currentID++;
}

void ChunkManager::meshChunk(Chunk &chunk, const glm::vec3 &position) {
    chunk.vertices = {};
    chunk.indices = {};
    std::array<bool, 6> facesToDraw{};
    const auto chunkExtent = glm::ivec3(position.x - 0.5f, position.y - 0.5f, position.z - 0.5f);
    const InternalNode *chunkRoot = dynamic_cast<InternalNode *>(chunk.octree);

    const int xStart = static_cast<int>(position.x - CHUNK_SHIFT);
    const int yStart = static_cast<int>(position.y - CHUNK_SHIFT);
    const int zStart = static_cast<int>(position.z - CHUNK_SHIFT);
    for (int x = xStart; x < xStart + CHUNK_SIZE; x++) {
        for (int z = zStart; z < zStart + CHUNK_SIZE; z++) {
            for (int y = yStart; y < yStart + CHUNK_SIZE; y++) {
                const Block block = getBlock(chunkRoot, x, y, z);
                if (block.color[3] != 0) {
                    generateBlockMesh(chunk, chunkExtent, block, facesToDraw);
                }
            }
        }
    }
    chunk.geometryModified = false;
}

void ChunkManager::generateBlockMesh(Chunk &chunk, const glm::ivec3 &chunkExtent, Block block,
                                     std::array<bool, 6> &facesToDraw) {
    std::fill(facesToDraw.begin(), facesToDraw.end(), false);

    const int xPos = (block.position[0] < 0 ? block.position[0] + 1 : block.position[0]) + chunkExtent.x;
    const int yPos = (block.position[1] < 0 ? block.position[1] + 1 : block.position[1]) + chunkExtent.y;
    const int zPos = (block.position[2] < 0 ? block.position[2] + 1 : block.position[2]) + chunkExtent.z;
    auto adjustedPosition = glm::vec3(xPos, yPos, zPos);

    int faceCount = 0;
    if (!hasBlock(adjustedPosition + glm::vec3(0, 1, 0))) {
        facesToDraw[0] = true;
        faceCount++;
    }
    if (!hasBlock(adjustedPosition + glm::vec3(0, -1, 0))) {
        facesToDraw[1] = true;
        faceCount++;
    }
    if (!hasBlock(adjustedPosition + glm::vec3(0, 0, 1))) {
        facesToDraw[2] = true;
        faceCount++;
    }
    if (!hasBlock(adjustedPosition + glm::vec3(0, 0, -1))) {
        facesToDraw[3] = true;
        faceCount++;
    }
    if (!hasBlock(adjustedPosition + glm::vec3(-1, 0, 0))) {
        facesToDraw[4] = true;
        faceCount++;
    }
    if (!hasBlock(adjustedPosition + glm::vec3(1, 0, 0))) {
        facesToDraw[5] = true;
        faceCount++;
    }

    if (faceCount > 0) {
        insertBlockIndices(chunk.indices, facesToDraw, chunk.vertices.size());
        insertBlockVertices(chunk.vertices, facesToDraw, adjustedPosition, block.color);
    }
}

void ChunkManager::addBlock(const glm::vec3 &position, const Block &block) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(position);
    Chunk *chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        createChunk(chunkCenter);
        chunk = getChunk(chunkCenter);
    }

    Block *newBlock = createPathToBlock(chunk, position);
    newBlock->position[0] = static_cast<int8_t>(round(position.x - chunkCenter.x));
    newBlock->position[1] = static_cast<int8_t>(round(position.y - chunkCenter.y));
    newBlock->position[2] = static_cast<int8_t>(round(position.z - chunkCenter.z));
    newBlock->color[0] = block.color[0];
    newBlock->color[1] = block.color[1];
    newBlock->color[2] = block.color[2];
    newBlock->color[3] = block.color[3];
    chunk->geometryModified = true;
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

Block ChunkManager::getBlock(const InternalNode *treeRoot, const int x, const int y, const int z) {
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

bool ChunkManager::hasBlock(const glm::vec3 &worldPos) {
    const OctreeNode *blockTree = findOctreeNode(worldPos);
    return blockTree != nullptr;
}

void ChunkManager::removeBlock(const glm::vec3 &worldPos) { //todo testing
    OctreeNode *blockTree = findOctreeNode(worldPos);

    if (blockTree == nullptr) {
        throw std::runtime_error("error removing block!");
    }

    blockTree->block = {};

    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);

    if (Chunk *chunk = getChunk(chunkCenter); chunk != nullptr) {
        chunk->geometryModified = true;
    }
}

void ChunkManager::fillChunk(const glm::vec3 &worldPos, const glm::vec3 &position, const Block &block) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);

    if (const Chunk *chunk = getChunk(chunkCenter); chunk == nullptr) {
        createChunk(chunkCenter);
    }

    const glm::vec3 chunkCorner = chunkCenter - CHUNK_SHIFT;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
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

    int depth = 0;
    while (depth < MAX_DEPTH) {
        int childIndex = 0;
        if (worldPos.x >= currentNode->position.x) childIndex |= 1;
        if (worldPos.y >= currentNode->position.y) childIndex |= 2;
        if (worldPos.z >= currentNode->position.z) childIndex |= 4;

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
    for (auto &[position, chunk]: chunks) {
        if (chunk.geometryModified) {
            TimeManager::startTimer("meshChunk");
            meshChunk(chunk, position);
            TimeManager::addTimeToProfiler("meshChunk", TimeManager::finishTimer("meshChunk"));

            TimeManager::startTimer("addToVertexPool");
            if (!chunk.vertices.empty()) {
                VertexPool::addToVertexPool(chunk.vertices, chunk.indices, chunk.ID);
            }
            TimeManager::addTimeToProfiler("addToVertexPool", TimeManager::finishTimer("addToVertexPool"));
        }
    }
}

uint32_t ChunkManager::chunkCount() const {
    return chunks.size();
}
