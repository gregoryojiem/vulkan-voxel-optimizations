#ifndef CHUNK_H
#define CHUNK_H
#include <array>
#include <vector>

#include "Block.h"
#include "../rendering/misc/Vertex.h"

constexpr int CHUNK_SIZE = 32;
constexpr int HALF_CHUNK_SIZE = CHUNK_SIZE/2;
constexpr float CHUNK_SHIFT = static_cast<float>(HALF_CHUNK_SIZE) - 0.5f;
constexpr int MAX_DEPTH = static_cast<int>(log2(CHUNK_SIZE));

template <int depth>
constexpr auto GET_DEPTH_SUBDIVISIONS() {
    std::array<float, depth> increments{};
    for (int i = 0; i < depth; i++) {
        increments[i] = static_cast<float>(CHUNK_SIZE) / std::pow(2, i + 2);
    }
    return increments;
}

constexpr auto DEPTH_SUBDIVISIONS = GET_DEPTH_SUBDIVISIONS<MAX_DEPTH>();

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
