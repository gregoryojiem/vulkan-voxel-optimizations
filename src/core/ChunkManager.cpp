#include "ChunkManager.h"

#include <iostream>
#include <stdexcept>
#include <glm/common.hpp>

#include "../rendering/scene/VertexPool.h"
#include "../util/VertexUtil.h"
#include "../util/TimeManager.h"

uint32_t ChunkManager::currentID = 1;

Chunk* ChunkManager::getChunk(const glm::vec3& worldPos) {
    auto it = chunks.find(Chunk::alignToChunkPos(worldPos));
    if (it != chunks.end()) {
        return &it->second;
    }
    return nullptr;
}

void ChunkManager::createChunk(const glm::vec3& worldPos) {
    glm::vec3 normalizedChunkPos = (worldPos - CHUNK_SHIFT) / CHUNK_SIZE;

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

void ChunkManager::meshChunk(Chunk& chunk) {
    chunk.vertices = { };
    chunk.indices = { };
    std::array<bool, 6> facesToDraw{};

    for (const auto& topNode : dynamic_cast<InternalNode*>(chunk.octree)->children) {
        if (topNode == nullptr) {
            continue;
        }
        for (const auto& middleNode : dynamic_cast<InternalNode*>(topNode)->children) {
            if (middleNode == nullptr) {
                continue;
            }
            for (const auto blockNode : dynamic_cast<InternalNode*>(middleNode)->children) {
                if (blockNode != nullptr) {
                    generateBlockMesh(chunk, blockNode->block, facesToDraw);
                }
            }
        }
    }

    chunk.geometryModified = false;
}

void ChunkManager::generateBlockMesh(Chunk& chunk, Block& block, std::array<bool, 6>& facesToDraw) {
    std::fill(facesToDraw.begin(), facesToDraw.end(), true);

    int faceCount = 0;
    if (hasBlock(block.position + glm::vec3(0, 1, 0))) {
        facesToDraw[0] = false;
        faceCount++;
    }
    if (hasBlock(block.position + glm::vec3(0, -1, 0))) {
        facesToDraw[1] = false;
        faceCount++;
    }
    if (hasBlock(block.position + glm::vec3(0, 0, 1))) {
        facesToDraw[2] = false;
        faceCount++;
    }
    if (hasBlock(block.position + glm::vec3(0, 0, -1))) {
        facesToDraw[3] = false;
        faceCount++;
    }
    if (hasBlock(block.position + glm::vec3(-1, 0, 0))) {
        facesToDraw[4] = false;
        faceCount++;
    }
    if (hasBlock(block.position + glm::vec3(1, 0, 0))) {
        facesToDraw[5] = false;
        faceCount++;
    }

    if (faceCount > 0) {
        insertBlockIndices(chunk.indices, facesToDraw, chunk.vertices.size());
        insertBlockVertices(chunk.vertices, facesToDraw, block.position, block.color);
    }
}

void ChunkManager::addBlock(const Block& block) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(block.position);
    Chunk* chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        createChunk(chunkCenter);
        chunk = getChunk(chunkCenter);
    }

    OctreeNode* newBlockNode = createPathToBlock(chunk, block);
    Block::copyBlock(newBlockNode->block, block);

    chunk->geometryModified = true;
}

OctreeNode* ChunkManager::createPathToBlock(const Chunk* chunk, const Block& block) {
    auto* currentNode = dynamic_cast<InternalNode*>(chunk->octree);
    OctreeNode* newBlockNode = nullptr;
    int depth = 0;

    while (depth < MAX_DEPTH) {
        const int octantIndex = Chunk::getOctantIndex(block.position, currentNode->block.position);

        if (depth < MAX_DEPTH - 1 && currentNode->children[octantIndex] != nullptr) {
            currentNode = dynamic_cast<InternalNode*>(currentNode->children[octantIndex]);
            depth++;
            continue;
        }

        if (depth == MAX_DEPTH && currentNode->children[octantIndex] != nullptr) {
            newBlockNode = currentNode->children[octantIndex];
            break;
        }

        glm::vec3 childPos = currentNode->block.position;
        Chunk::addOctantOffset(childPos, octantIndex, depth);

        if (depth < MAX_DEPTH - 1) {
            currentNode->children[octantIndex] = new InternalNode(childPos);
            currentNode = dynamic_cast<InternalNode*>(currentNode->children[octantIndex]);
        }
        else {
            currentNode->children[octantIndex] = new OctreeNode();
            newBlockNode = currentNode->children[octantIndex];
        }

        depth++;
    }

    if (newBlockNode == nullptr) {
        throw std::runtime_error("failed to add block!");
    }

    return newBlockNode;
}

Block ChunkManager::getBlock(const glm::vec3& worldPos) {
    const OctreeNode* blockTree = findOctreeNode(worldPos);

    if (blockTree == nullptr) {
        throw std::runtime_error("error getting block!");
    }

    return blockTree->block;
}

bool ChunkManager::hasBlock(const glm::vec3& worldPos) {
    OctreeNode* blockTree = findOctreeNode(worldPos);
    return blockTree != nullptr;
}

void ChunkManager::removeBlock(const glm::vec3& worldPos) { //todo remove geometry
    OctreeNode* blockTree = findOctreeNode(worldPos);

    if (blockTree == nullptr) {
        throw std::runtime_error("error removing block!");
    }

    blockTree->block = {};

    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);

    if (Chunk* chunk = getChunk(chunkCenter); chunk != nullptr) {
        chunk->geometryModified = true;
    }
}

void ChunkManager::fillChunk(const glm::vec3 &worldPos, Block block) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);

    if (const Chunk* chunk = getChunk(chunkCenter); chunk == nullptr) {
        createChunk(chunkCenter);
    }

    const glm::vec3 chunkCorner = chunkCenter - 3.5f;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                Block newBlock = block;
                newBlock.position = chunkCorner + glm::vec3(i, j, k);
                addBlock(newBlock);
            }
        }
    }
}

OctreeNode* ChunkManager::findOctreeNode(const glm::vec3& worldPos) {
    const glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);
    const Chunk* chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        return nullptr;
    }

    auto* currentNode = dynamic_cast<InternalNode*>(chunk->octree);

    int depth = 0;
    while (depth < MAX_DEPTH) {
        int childIndex = 0;
        if (worldPos.x >= currentNode->block.position.x) childIndex |= 1;
        if (worldPos.y >= currentNode->block.position.y) childIndex |= 2;
        if (worldPos.z >= currentNode->block.position.z) childIndex |= 4;

        if (currentNode->children[childIndex] == nullptr) {
            return nullptr;
        }

        if (depth == MAX_DEPTH - 1) {
            return currentNode->children[childIndex];
        }

        currentNode = dynamic_cast<InternalNode*>(currentNode->children[childIndex]);
        depth++;
    }

    return currentNode;
}

void ChunkManager::meshAllChunks() {
    for (auto& [pos, chunk] : chunks) {
        if (chunk.geometryModified) {
            TimeManager::startTimer("meshChunk");
            meshChunk(chunk);
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
