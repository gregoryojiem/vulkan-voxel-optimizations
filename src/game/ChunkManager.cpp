#include "ChunkManager.h"

#include <stdexcept>
#include <glm/common.hpp>
#include "../utility/GraphicsUtil.h"
#include "../rendering/WorldGeometry.h"

std::unordered_map<glm::vec3, Chunk*> ChunkManager::chunks;

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

    std::vector<Vertex> blockVertices = generateBlockVertices(block);
    std::vector<uint32_t> blockIndices = generateBlockIndices(chunk->vertices.size());

    chunk->vertices.insert(chunk->vertices.end(), blockVertices.begin(), blockVertices.end());
    chunk->indices.insert(chunk->indices.end(), blockIndices.begin(), blockIndices.end());
}

Block* ChunkManager::getBlock(const glm::vec3& worldPos) {
    OctreeNode* blockTree = findOctreeNode(worldPos);

    if (blockTree == nullptr) {
        throw std::runtime_error("error adding new block!");
    }

    return blockTree->block;
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

void ChunkManager::saveChunkGeometry() {
    uint32_t indexOffset = 0;

    globalChunkVertices = { };
    globalChunkIndices = { };
    for (auto& chunkPair : chunks) {
        Chunk* chunk = chunkPair.second;

        globalChunkVertices.insert(globalChunkVertices.end(), chunk->vertices.begin(), chunk->vertices.end());

        for (auto& index : chunk->indices) {
            globalChunkIndices.push_back(index + indexOffset);
        }

        indexOffset += (uint32_t)chunk->vertices.size();
    }
}