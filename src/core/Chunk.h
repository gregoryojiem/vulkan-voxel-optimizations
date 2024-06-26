#ifndef CHUNK_H
#define CHUNK_H
#include <array>
#include <vector>

#include "Block.h"
#include "../rendering/scene/Vertex.h"

constexpr int CHUNK_SIZE = 32;
constexpr int CHUNK_SIZE_2 = CHUNK_SIZE * CHUNK_SIZE;
constexpr int CHUNK_SIZE_3 = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
constexpr int HALF_CHUNK_SIZE = CHUNK_SIZE/2;
constexpr int MAX_DEPTH = static_cast<uint32_t>(log2(CHUNK_SIZE));
constexpr float CHUNK_SHIFT = static_cast<float>(HALF_CHUNK_SIZE) - 0.5f;
constexpr int CHUNK_SIZE_PADDED = CHUNK_SIZE + 2;
constexpr int CHUNK_SIZE_PADDED_2 = CHUNK_SIZE_PADDED * CHUNK_SIZE_PADDED;
constexpr int CHUNK_SIZE_PADDED_3 = CHUNK_SIZE_PADDED * CHUNK_SIZE_PADDED * CHUNK_SIZE_PADDED;
constexpr int MAX_QUADS = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE / 2 * 6;

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
    glm::vec3 position{};
    bool isLeafNode{};

    union {
        OctreeNode *children[8]{};
        Block blocks[8];
    };

    explicit OctreeNode(const glm::vec3 &nodePosition);
    ~OctreeNode();
};

struct Chunk {
    OctreeNode *octree;
    bool notMeshed;
    uint32_t ID;

    ~Chunk();

    static glm::vec3 alignToChunkPos(const glm::vec3 &position);

    static glm::vec3 alignToChunkPos(int x, int y, int z);

    static double alignNum(double number);

    static int getOctantIndex(const glm::vec3 &blockPosition, const glm::vec3 &chunkPos);

    static void addOctantOffset(glm::vec3 &middlePosition, int octantIndex, int depth);

    static glm::vec3 calculateCorner(glm::vec3 position);
};

#endif //CHUNK_H
