#include "Chunk.h"

std::vector<float> SUB_INCREMENTS = { 2.0f, 1.0, 0.5 };
float CHUNK_SHIFT = 3.5;
glm::vec3 CHUNK_SIZE = glm::vec3(8.0f);
int MAX_DEPTH = 3;

OctreeNode::~OctreeNode() = default;

InternalNode::InternalNode(const glm::vec3& position) {
    for (auto& i : children) {
        i = nullptr;
    }
    block = Block(position);
}

InternalNode::~InternalNode() {
    for (const auto& i : children) {
        delete i;
    }
}

Chunk::~Chunk() {
    delete octree;
}

glm::vec3 Chunk::alignToChunkPos(const glm::vec3& position) {
    return {alignNum(position.x), alignNum(position.y), alignNum(position.z)};
}

double Chunk::alignNum(const double number) {
    return round((number - CHUNK_SHIFT) / 8) * 8 + CHUNK_SHIFT;
}

int Chunk::getOctantIndex(const glm::vec3& blockPos, const glm::vec3& chunkPos) {
    int childIndex = 0;
    if (blockPos.x >= chunkPos.x) childIndex |= 1;
    if (blockPos.y >= chunkPos.y) childIndex |= 2;
    if (blockPos.z >= chunkPos.z) childIndex |= 4;
    return childIndex;
}

void Chunk::addOctantOffset(glm::vec3& middlePosition, int octantIndex, int depth) {
    const float xSign = octantIndex & 1 ? 1 : -1;
    const float ySign = octantIndex & 2 ? 1 : -1;
    const float zSign = octantIndex & 4 ? 1 : -1;

    middlePosition.x += xSign * SUB_INCREMENTS[depth];
    middlePosition.y += SUB_INCREMENTS[depth] * ySign;
    middlePosition.z += SUB_INCREMENTS[depth] * zSign;
}