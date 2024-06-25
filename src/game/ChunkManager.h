#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "Block.h"
#include "Chunk.h"

// hash function for vec3s so they can be used in the unordered map of ChunkManager
template<>
struct std::hash<glm::vec3> {
    std::size_t operator()(const glm::vec3 &v) const noexcept {
        return std::hash<float>()(v.x) ^
               std::hash<float>()(v.y) ^
               std::hash<float>()(v.z);
    }
};

class ChunkManager {
public:
    std::unordered_map<glm::vec3, Chunk> chunks;
    static uint32_t currentID;

    Chunk *getChunk(const glm::vec3 &worldPos);

    void createChunk(const glm::vec3 &worldPos);

    void fillChunk(const glm::vec3 &worldPos, const glm::vec3 &position, const Block &block);

    void meshChunk(ChunkVertex chunkVertices[MAX_QUADS], Chunk &chunk, const glm::vec3 &position,
                   uint32_t &vertexStart);

    static void setInternalBits(uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                Block chunkBlockInfo[CHUNK_SIZE_3], const InternalNode *chunkRoot);

    static void setAdjacentChunkBits(uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                                     const InternalNode *chunkRoot, int xStart, int xEnd, int yStart, int yEnd,
                                     int zStart, int zEnd, int xOffset, int yOffset, int zOffset);

    void setExternalBits(uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                         const glm::ivec3 &chunkCorner);

    static void cullBlockFaces(uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
                               uint64_t chunkBitsPerAxis[3][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED]);

    static void generateBinaryPlanes(
        std::array<std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::array<uint32_t, CHUNK_SIZE> > >, 6>
        &planesPerBlock, uint64_t culledFaces[6][CHUNK_SIZE_PADDED][CHUNK_SIZE_PADDED],
        Block chunkBlockInfo[CHUNK_SIZE_3]);

    static void meshAllBinaryPlanes(ChunkVertex chunkVertices[MAX_QUADS],
                                    std::array<std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::array<
                                        uint32_t, CHUNK_SIZE> > >, 6> &data,
                                    ChunkVertex &vertexInfo, uint32_t &vertexStart, const glm::ivec3 &chunkCorner);

    static void greedyMeshBinaryPlane(ChunkVertex chunkVertices[MAX_QUADS], uint32_t &vertexStart,
                                      std::array<uint32_t, CHUNK_SIZE> &plane,
                                      const glm::ivec3 &chunkCorner, ChunkVertex &vertexInfo, uint32_t axis,
                                      uint32_t axisPos);

    void meshAllChunks();

    uint32_t chunkCount() const;

    void addBlock(const glm::vec3 &position, const Block &block);

    static Block *createPathToBlock(const Chunk *chunk, const glm::vec3 &position);

    static Block getBlock(const InternalNode *treeRoot, int x, int y, int z);

    Block getBlock(int x, int y, int z);

    bool hasBlock(const glm::vec3 &worldPos);

    void removeBlock(const glm::vec3 &worldPos);

    void generateBlockMesh(Chunk &chunk, const glm::ivec3 &chunkCorner, Block block, int x, int y, int z);

private:
    OctreeNode *findOctreeNode(const glm::vec3 &worldPos);
};

#endif //CHUNKMANAGER_H
