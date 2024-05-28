#ifndef CHUNK_H
#define CHUNK_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "Block.h"

struct Vertex;

struct OctreeNode {
    OctreeNode* children[8] = {nullptr};
    Block* block;

    OctreeNode();
    ~OctreeNode();

    explicit OctreeNode(const glm::vec3& position);
};

struct Chunk {
    OctreeNode* octree;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    bool geometryModified;

    explicit Chunk(const glm::vec3& pos);
    ~Chunk();

    static glm::vec3 alignToChunkPos(const glm::vec3& position);
    static double alignNum(double number);
};

// hash function for vec3s so they can be used in the unordered map of ChunkManager
namespace std {
    template<>
    struct hash<glm::vec3> {
        std::size_t operator()(const glm::vec3& v) const {
            return std::hash<float>()(v.x) ^
                   std::hash<float>()(v.y) ^
                   std::hash<float>()(v.z);
        }
    };
}

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    Chunk* getChunk(const glm::vec3& worldPos);
    void createChunk(const glm::vec3& worldPos);
    void addBlock(const Block& block);
    Block* getBlock(const glm::vec3& worldPos);
    void removeBlock(const glm::vec3& worldPos);
    void fillChunk(const glm::vec3& worldPos, Block block);
    void saveChunkGeometry();

private:
    std::unordered_map<glm::vec3, Chunk*> chunks;

    OctreeNode* findOctreeNode(const glm::vec3& worldPos);
};

static glm::vec3 chunkSize = glm::vec3(8.0f);
static std::vector<float> subIncrements = { 2.0f, 1.0, 0.5 };
static float chunkShift = 3.5;
static int maxDepth = 3;
#endif //CHUNK_H
