#include "GraphicsUtil.h"

#include "../rendering/Vertex.h"

std::vector<ChunkVertex> generateBlockVertices(Block block) {
    const std::vector<ChunkVertex> geometry = {
        {{0.5f + block.position.x, 0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, 0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{0.5f + block.position.x, 0.5f + block.position.y, -0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, 0.5f + block.position.y, -0.5f + block.position.z}, block.color},

        {{0.5f + block.position.x, -0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, -0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{0.5f + block.position.x, -0.5f + block.position.y, -0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, -0.5f + block.position.y, -0.5f + block.position.z}, block.color},
    };

    return geometry;
}

std::vector<uint32_t> generateBlockIndices(uint32_t startIndex) {
    const std::vector indices = {
        //top face (facing (0, -1, 0))
        startIndex + 0u, startIndex + 2u, startIndex + 1u,
        startIndex + 3u, startIndex + 1u, startIndex + 2u,

        //bottom face
        startIndex + 7u, startIndex + 6u, startIndex + 5u,
        startIndex + 4u, startIndex + 5u, startIndex + 6u,

        //front face (facing (0, 0, -1))
        startIndex + 1u, startIndex + 5u, startIndex + 0u,
        startIndex + 4u, startIndex + 0u, startIndex + 5u,

        //back face
        startIndex + 3u, startIndex + 2u, startIndex + 7u,
        startIndex + 6u, startIndex + 7u, startIndex + 2u,

        //left face (facing (-1, 0, 0))
        startIndex + 7u, startIndex + 5u, startIndex + 3u,
        startIndex + 1u, startIndex + 3u, startIndex + 5u,

        //right face
        startIndex + 0u, startIndex + 4u, startIndex + 2u,
        startIndex + 6u, startIndex + 2u, startIndex + 4u
    };

    return indices;
}
