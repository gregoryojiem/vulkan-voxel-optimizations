#ifndef CHUNK_H
#define CHUNK_H
#include <vector>

#include "Block.h"
#include "../rendering/misc/Vertex.h"

extern std::vector<float> SUB_INCREMENTS;
extern int MAX_DEPTH;
extern float CHUNK_SHIFT;
extern int CHUNK_SIZE;
extern int HALF_CHUNK_SIZE;

struct OctreeNode {
    Block block{};

    virtual ~OctreeNode();
};

struct InternalNode final : OctreeNode {
    glm::vec3 position{};
    OctreeNode *children[8] = {nullptr};

    explicit InternalNode(const glm::vec3 &nodePosition);

    ~InternalNode() override;
};

struct Chunk {
    OctreeNode *octree;
    std::vector<ChunkVertex> vertices;
    bool geometryModified;
    uint32_t ID;

    ~Chunk();

    static glm::vec3 alignToChunkPos(const glm::vec3 &position);

    static glm::vec3 alignToChunkPos(int x, int y, int z);

    static double alignNum(double number);

    static int getOctantIndex(const glm::vec3 &blockPosition, const glm::vec3 &chunkPos);

    static void addOctantOffset(glm::vec3 &middlePosition, int octantIndex, int depth);
};

#endif //CHUNK_H
