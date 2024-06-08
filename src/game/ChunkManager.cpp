#include "ChunkManager.h"

#include <iostream>
#include <stdexcept>
#include <glm/common.hpp>

#include "../rendering/VertexPool.h"
#include "../utility/GraphicsUtil.h"
#include "../utility/TimeManager.h"
#include "../rendering/Vertex.h"

std::unordered_map<glm::vec3, Chunk*> ChunkManager::chunks;
uint32_t ChunkManager::currentID = 1;

OctreeNode::OctreeNode() {
    for (int i = 0; i < 8; ++i) {
        children[i] = nullptr;
    }
    block = nullptr;
}

OctreeNode::OctreeNode(const glm::vec3& position) {
    for (int i = 0; i < 8; ++i) {
        children[i] = nullptr;
    }
    block = new Block(position);
}

OctreeNode::~OctreeNode() {
    for (int i = 0; i < 8; ++i) {
        delete children[i];
    }
    delete block;
}

Chunk::Chunk(const glm::vec3& pos) {
    octree = new OctreeNode(pos);
    geometryModified = false;
    ID = ChunkManager::currentID++;
}

Chunk::~Chunk() {
    delete octree;
}

glm::vec3 Chunk::alignToChunkPos(const glm::vec3& position) {
    return {alignNum(position.x), alignNum(position.y), alignNum(position.z)};
}

double Chunk::alignNum(double number) {
    return round((number - chunkShift) / 8) * 8 + chunkShift;
}

ChunkManager::ChunkManager() = default;

ChunkManager::~ChunkManager() {
    for (auto& pair : chunks) {
        delete pair.second;
    }
}

Chunk* ChunkManager::getChunk(const glm::vec3& worldPos) {
    auto it = chunks.find(Chunk::alignToChunkPos(worldPos));
    if (it != chunks.end()) {
        return it->second;
    }
    return nullptr;
}

void ChunkManager::createChunk(const glm::vec3& worldPos) {
    glm::vec3 normalizedChunkPos = (worldPos - chunkShift) / chunkSize;

    if (std::floor(normalizedChunkPos.x) != normalizedChunkPos.x ||
        std::floor(normalizedChunkPos.y) != normalizedChunkPos.y ||
        std::floor(normalizedChunkPos.z) != normalizedChunkPos.z) {
        throw std::runtime_error("chunk creation error: location invalid!");
    }

    if (chunks.find(worldPos) != chunks.end()) {
        throw std::runtime_error("chunk creation error: chunk already exists!");
    }

    auto newChunk = new Chunk(worldPos);
    chunks[worldPos] = newChunk;
}

void ChunkManager::meshChunk(Chunk& chunk) {
    chunk.vertices = { };
    chunk.indices = { };
    std::array<bool, 6> facesToDraw;

    for (auto& topNode : chunk.octree->children) {
        if (topNode == nullptr) {
            continue;
        }
        for (auto& middleNode : topNode->children) {
            if (middleNode == nullptr) {
                continue;
            }
            for (auto blockNode : middleNode->children) {
                if (blockNode != nullptr) {
                    generateBlockMesh(chunk, blockNode->block, facesToDraw);
                }
            }
        }
    }

    chunk.geometryModified = false;
}

void ChunkManager::generateBlockMesh(Chunk& chunk, Block* block, std::array<bool, 6>& facesToDraw) {
    std::fill(facesToDraw.begin(), facesToDraw.end(), true);

    int faceCount = 0;
    if (hasBlock(block->position + glm::vec3(0, 1, 0))) {
        facesToDraw[0] = false;
        faceCount++;
    }
    if (hasBlock(block->position + glm::vec3(0, -1, 0))) {
        facesToDraw[1] = false;
        faceCount++;
    }
    if (hasBlock(block->position + glm::vec3(0, 0, 1))) {
        facesToDraw[2] = false;
        faceCount++;
    }
    if (hasBlock(block->position + glm::vec3(0, 0, -1))) {
        facesToDraw[3] = false;
        faceCount++;
    }
    if (hasBlock(block->position + glm::vec3(-1, 0, 0))) {
        facesToDraw[4] = false;
        faceCount++;
    }
    if (hasBlock(block->position + glm::vec3(1, 0, 0))) {
        facesToDraw[5] = false;
        faceCount++;
    }

    if (faceCount > 0) {
        insertBlockIndices(chunk.indices, facesToDraw, chunk.vertices.size());
        insertBlockVertices(chunk.vertices, facesToDraw, block);
    }
}

void ChunkManager::addBlock(const Block& block) {
    glm::vec3 chunkCenter = Chunk::alignToChunkPos(block.position);
    Chunk* chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        createChunk(chunkCenter);
        chunk = getChunk(chunkCenter);
    }

    OctreeNode* currentNode = chunk->octree;
    int depth = 0;
    while (depth < maxDepth) {
        int childIndex = 0;
        if (block.position.x >= currentNode->block->position.x) childIndex |= 1;
        if (block.position.y >= currentNode->block->position.y) childIndex |= 2;
        if (block.position.z >= currentNode->block->position.z) childIndex |= 4;

        if (currentNode->children[childIndex] == nullptr) {
            currentNode->children[childIndex] = new OctreeNode();
            glm::vec3 childPos = currentNode->block->position;

            const int xSign = childIndex & 1 ? 1 : -1;
            const int ySign = childIndex & 2 ? 1 : -1;
            const int zSign = childIndex & 4 ? 1 : -1;

            childPos.x += subIncrements[depth] * xSign;
            childPos.y += subIncrements[depth] * ySign;
            childPos.z += subIncrements[depth] * zSign;
            currentNode->children[childIndex]->block = new Block(childPos);
        }

        currentNode = currentNode->children[childIndex];
        depth++;
    }

    currentNode->block = new Block(block);
    chunk->geometryModified = true;
}

Block* ChunkManager::getBlock(const glm::vec3& worldPos) {
    OctreeNode* blockTree = findOctreeNode(worldPos);

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

    blockTree->block = nullptr;

    glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);
    Chunk* chunk = getChunk(chunkCenter);

    if (chunk != nullptr) {
        chunk->geometryModified = true;
    }
}

void ChunkManager::fillChunk(const glm::vec3 &worldPos, Block block) {
    glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);
    Chunk* chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        createChunk(chunkCenter);
        chunk = getChunk(chunkCenter);
    }

    glm::vec3 chunkCorner = chunkCenter - 3.5f;
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
    glm::vec3 chunkCenter = Chunk::alignToChunkPos(worldPos);
    Chunk* chunk = getChunk(chunkCenter);

    if (chunk == nullptr) {
        return nullptr;
    }

    OctreeNode* currentNode = chunk->octree;
    int depth = 0;
    while (depth < maxDepth) {
        int childIndex = 0;
        if (worldPos.x >= currentNode->block->position.x) childIndex |= 1;
        if (worldPos.y >= currentNode->block->position.y) childIndex |= 2;
        if (worldPos.z >= currentNode->block->position.z) childIndex |= 4;

        if (currentNode->children[childIndex] == nullptr) {
            return nullptr;
        }

        currentNode = currentNode->children[childIndex];
        depth++;
    }

    return currentNode;
}

void ChunkManager::meshAllChunks() {
    for (auto& [pos, chunk] : chunks) {
        if (chunk->geometryModified) {
            TimeManager::startTimer("meshChunk");
            meshChunk(*chunk);
            TimeManager::addTimeToProfiler("meshChunk", TimeManager::finishTimer("meshChunk"));
            
            TimeManager::startTimer("addToVertexPool");
            if (!chunk->vertices.empty()) {
                VertexPool::addToVertexPool(*chunk);
            }
            TimeManager::addTimeToProfiler("addToVertexPool", TimeManager::finishTimer("addToVertexPool"));
        }
    }
}

size_t ChunkManager::chunkCount() {
    return chunks.size();
}
