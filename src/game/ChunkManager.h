#ifndef CHUNK_H
#define CHUNK_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "Block.h"
#include "../rendering/Vertex.h"

static glm::vec3 chunkSize = glm::vec3(8.0f);
static std::vector<float> subIncrements = { 2.0f, 1.0, 0.5 };
static float chunkShift = 3.5;
static int maxDepth = 3;

struct OctreeNode {
    OctreeNode* children[8] = {nullptr};
    Block* block;

    explicit OctreeNode(const glm::vec3& position);
    ~OctreeNode();
};

struct Chunk {
    OctreeNode* octree;
    std::vector<ChunkVertex> vertices;
    std::vector<uint32_t> indices;
    bool geometryModified;
    uint32_t ID;

    static glm::vec3 alignToChunkPos(const glm::vec3& position);
    static double alignNum(double number);
};

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
    size_t chunkCount() const;

    void addBlock(const Block& block);
    Block* getBlock(const glm::vec3& worldPos);
    bool hasBlock(const glm::vec3& worldPos);
    void removeBlock(const glm::vec3& worldPos);
    void generateBlockMesh(Chunk& chunk, Block* block, std::array<bool, 6>& facesToDraw);

private:
    OctreeNode* findOctreeNode(const glm::vec3& worldPos);
};

#endif //CHUNK_H
