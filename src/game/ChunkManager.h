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
    std::size_t operator()(const glm::vec3& v) const noexcept {
        return std::hash<float>()(v.x) ^
               std::hash<float>()(v.y) ^
               std::hash<float>()(v.z);
    }
};

class ChunkManager {
public:
    std::unordered_map<glm::vec3, Chunk> chunks;
    static uint32_t currentID;

    Chunk* getChunk(const glm::vec3& worldPos);
    void createChunk(const glm::vec3& worldPos);
    void fillChunk(const glm::vec3& worldPos, Block block);
    void meshChunk(Chunk& chunk);
    void meshAllChunks();
    uint32_t chunkCount() const;

    void addBlock(const Block& block);

    static OctreeNode* createPathToBlock(const Chunk* chunk, const Block& block);
    Block getBlock(const glm::vec3& worldPos);
    bool hasBlock(const glm::vec3& worldPos);
    void removeBlock(const glm::vec3& worldPos);
    void generateBlockMesh(Chunk& chunk, Block& block, std::array<bool, 6>& facesToDraw);

private:
    OctreeNode* findOctreeNode(const glm::vec3& worldPos);
};

#endif //CHUNKMANAGER_H
