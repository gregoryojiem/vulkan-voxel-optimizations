#include "VertexUtil.h"

#include "../rendering/scene/Vertex.h"

static constexpr std::array<std::array<glm::vec3, 4>, 6> facePositions = {
    glm::vec3{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
    glm::vec3{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f},
    glm::vec3{0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f},
    glm::vec3{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
    glm::vec3{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f},
    glm::vec3{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}
};

void insertBlockVertices(std::vector<ChunkVertex> &chunkVertices, const glm::vec3 &blockPos, uint8_t color[4],
                         int face) {
    for (const auto &pos: facePositions[face]) {
        ChunkVertex newVertex = {
            pos + blockPos,
            color[0],
            color[1],
            color[2]
        };
        chunkVertices.push_back(newVertex);
    }
}

void insertGreedyQuad(ChunkVertex chunkVertices[MAX_QUADS], uint32_t &vertexStart,
                      const ChunkVertex &vertexInfo, const glm::ivec3 &chunkCorner,
                      uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t axis, uint32_t axisPos) {
    glm::ivec3 pos{};
    if (axis == 0) {
        pos = {x, axisPos, y};
        pos += glm::ivec3(chunkCorner.x, chunkCorner.y, chunkCorner.z);
    } else if (axis == 1) {
        pos = {x, axisPos + 1, y};
        pos += glm::ivec3(chunkCorner.x, chunkCorner.y, chunkCorner.z);
    } else if (axis == 2) {
        pos = {axisPos, y, x};
        pos += glm::ivec3(chunkCorner.x, chunkCorner.y, chunkCorner.z);
    } else if (axis == 3) {
        pos = {axisPos + 1, y, x};
        pos += glm::ivec3(chunkCorner.x, chunkCorner.y, chunkCorner.z);
    } else if (axis == 4) {
        pos = {x, y, axisPos};
        pos += glm::ivec3(chunkCorner.x, chunkCorner.y, chunkCorner.z);
    } else if (axis == 5) {
        pos = {x, y, axisPos + 1};
        pos += glm::ivec3(chunkCorner.x, chunkCorner.y, chunkCorner.z);
    }

    if (axis == 0) {
        //bottom face
        chunkVertices[vertexStart] = {
            pos + glm::ivec3{width, 0, height}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 1] = {
            pos + glm::ivec3{width, 0, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 2] = {
            pos + glm::ivec3{0, 0, height}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 3] = {pos, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]};
    }
    else if (axis == 1) {
        //top face
        chunkVertices[vertexStart] = {
            pos + glm::ivec3{width, 0, height}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 1] = {
            pos + glm::ivec3{0, 0, height}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 2] = {
            pos + glm::ivec3{width, 0, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 3] = {pos, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]};
    }
    else if (axis == 2) {
        //left face
        chunkVertices[vertexStart] = {
            pos + glm::ivec3{0, height, width}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 1] = {
            pos + glm::ivec3{0, 0, width}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 2] = {
            pos + glm::ivec3{0, height, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 3] = {pos, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]};
    }
    else if (axis == 3) {
        //right face
        chunkVertices[vertexStart] = {
            pos + glm::ivec3{0, height, width}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 1] = {
            pos + glm::ivec3{0, height, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 2] = {
            pos + glm::ivec3{0, 0, width}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 3] = {pos, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]};
    }
    else if (axis == 4) {
        //back face
        chunkVertices[vertexStart] = {
            pos + glm::ivec3{width, height, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 1] = {
            pos + glm::ivec3{0, height, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 2] = {
            pos + glm::ivec3{width, 0, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 3] = {pos, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]};
    }
    else if (axis == 5) {
        //front face
        chunkVertices[vertexStart] = {
            pos + glm::ivec3{width, height, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 1] = {
            pos + glm::ivec3{width, 0, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 2] = {
            pos + glm::ivec3{0, height, 0}, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]
        };
        chunkVertices[vertexStart + 3] = {pos, vertexInfo.color[0], vertexInfo.color[1], vertexInfo.color[2]};
    }
    vertexStart += 4;
}

std::vector<TexturedVertex> generateTexturedQuad(glm::vec4 quadBounds, glm::vec4 texQuadBounds, glm::vec2 startPos) {
    float left = quadBounds[0];
    float bottom = quadBounds[1];
    float right = quadBounds[2];
    float top = quadBounds[3];

    // top and bottom switched due to y format differences
    float texLeft = texQuadBounds[0];
    float texTop = texQuadBounds[1];
    float texRight = texQuadBounds[2];
    float texBottom = texQuadBounds[3];

    return {
        {
            {startPos.x + left, startPos.y - bottom},
            {1.0f, 0.0f, 0.0f},
            {texLeft, texTop}
        },

        {
            {startPos.x + right, startPos.y - bottom},
            {0.0f, 1.0f, 0.0f},
            {texRight, texTop}
        },

        {
            {startPos.x + right, startPos.y - top},
            {0.0f, 0.0f, 1.0f},
            {texRight, texBottom}
        },

        {
            {startPos.x + left, startPos.y - top},
            {1.0f, 1.0f, 1.0f},
            {texLeft, texBottom}
        }
    };
}

std::vector<uint32_t> generateTexturedQuadIndices(uint32_t startIndex) {
    return {
        startIndex + 0, startIndex + 1, startIndex + 2,
        startIndex + 3, startIndex + 0, startIndex + 2
    };;
}
