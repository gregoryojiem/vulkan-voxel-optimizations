#include "GraphicsUtil.h"

#include "../rendering/misc/Vertex.h"

static constexpr std::array<std::array<glm::vec3, 4>, 6>  facePositions = {
    glm::vec3{0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
    glm::vec3{0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f},
    glm::vec3{-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f},
    glm::vec3{-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
    glm::vec3{-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    glm::vec3{0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, -0.5f}
};

void insertBlockVertices(std::vector<ChunkVertex> &chunkVertices, const glm::vec3 &blockPos, uint8_t color[4], int face) {
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
