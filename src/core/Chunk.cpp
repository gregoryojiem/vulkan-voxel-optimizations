#include "Chunk.h"

OctreeNode::OctreeNode(const glm::vec3 &nodePosition) : position(nodePosition) {
}

OctreeNode::~OctreeNode() {
    if (isLeafNode) {
        return;
    }
    for (const auto &i: children) {
        delete i;
    }
}

Chunk::~Chunk() {
    delete octree;
}

glm::vec3 Chunk::alignToChunkPos(const glm::vec3 &position) {
    return {alignNum(position.x), alignNum(position.y), alignNum(position.z)};
}

glm::vec3 Chunk::alignToChunkPos(int x, int y, int z) {
    return {alignNum(x), alignNum(y), alignNum(z)};
}

double Chunk::alignNum(const double number) {
    return round((number - CHUNK_SHIFT) / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SHIFT;
}

int Chunk::getOctantIndex(const glm::vec3 &blockPosition, const glm::vec3 &chunkPos) {
    int childIndex = 0;
    if (blockPosition.x >= chunkPos.x) childIndex |= 1;
    if (blockPosition.y >= chunkPos.y) childIndex |= 2;
    if (blockPosition.z >= chunkPos.z) childIndex |= 4;
    return childIndex;
}

void Chunk::addOctantOffset(glm::vec3 &middlePosition, int octantIndex, int depth) {
    const float xSign = octantIndex & 1 ? 1 : -1;
    const float ySign = octantIndex & 2 ? 1 : -1;
    const float zSign = octantIndex & 4 ? 1 : -1;

    middlePosition.x += xSign * DEPTH_SUBDIVISIONS[depth];
    middlePosition.y += DEPTH_SUBDIVISIONS[depth] * ySign;
    middlePosition.z += DEPTH_SUBDIVISIONS[depth] * zSign;
}

glm::vec3 Chunk::calculateCorner(const glm::vec3 position) {
    auto chunkCorner = glm::ivec3(position.x - HALF_CHUNK_SIZE, position.y - HALF_CHUNK_SIZE,
                                  position.z - HALF_CHUNK_SIZE);
    if (chunkCorner.x > 0) {
        chunkCorner.x += 1;
    }
    if (chunkCorner.y > 0) {
        chunkCorner.y += 1;
    }
    if (chunkCorner.z > 0) {
        chunkCorner.z += 1;
    }
    return chunkCorner;
}
