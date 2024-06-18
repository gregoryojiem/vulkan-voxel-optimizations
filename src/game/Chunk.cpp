#include "Chunk.h"

std::vector<float> SUB_INCREMENTS = {2.0f, 1.0, 0.5};
int MAX_DEPTH = 3;
float CHUNK_SHIFT = 3.5;
int CHUNK_SIZE = 8;
int HALF_CHUNK_SIZE = CHUNK_SIZE/2;

OctreeNode::~OctreeNode() = default;

InternalNode::InternalNode(const glm::vec3 &nodePosition) {
    position = nodePosition;
}

InternalNode::~InternalNode() {
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
    return round((number - CHUNK_SHIFT) / 8) * 8 + CHUNK_SHIFT;
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

    middlePosition.x += xSign * SUB_INCREMENTS[depth];
    middlePosition.y += SUB_INCREMENTS[depth] * ySign;
    middlePosition.z += SUB_INCREMENTS[depth] * zSign;
}
