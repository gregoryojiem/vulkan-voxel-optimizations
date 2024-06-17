#ifndef CHUNK_H
#define CHUNK_H
#include <vector>

#include "Block.h"
#include "../rendering/scene/Vertex.h"

extern std::vector<float> SUB_INCREMENTS;
extern float CHUNK_SHIFT;
extern glm::vec3 CHUNK_SIZE;
extern int MAX_DEPTH;

struct OctreeNode {
    Block block{};

    virtual ~OctreeNode();
};

struct InternalNode final : OctreeNode {
    OctreeNode *children[8] = {nullptr};

    explicit InternalNode(const glm::vec3 &position);

    ~InternalNode() override;
};

struct Chunk {
    OctreeNode *octree;
    std::vector<ChunkVertex> vertices;
    std::vector<uint32_t> indices;
    bool geometryModified;
    uint32_t ID;

    ~Chunk();

    static glm::vec3 alignToChunkPos(const glm::vec3 &position);

    static double alignNum(double number);

    static int getOctantIndex(const glm::vec3 &blockPos, const glm::vec3 &chunkPos);

    static void addOctantOffset(glm::vec3 &middlePosition, int octantIndex, int depth);
};

#endif //CHUNK_H
