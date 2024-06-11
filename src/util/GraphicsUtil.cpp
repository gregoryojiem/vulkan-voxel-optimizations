#include "GraphicsUtil.h"

#include "../rendering/Vertex.h"

void insertBlockVertices(std::vector<ChunkVertex>& chunkVertices, std::array<bool, 6>& facesToDraw, Block& block) {
    const std::array<glm::vec3, 8> positions = {
        {{0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
            {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}}
    };

    for (const auto& pos : positions) {
        ChunkVertex newVertex = {
            pos + block.position,
            block.color[0],
            block.color[1],
            block.color[2]};
        chunkVertices.push_back(newVertex);
    }
}

void insertBlockIndices(std::vector<uint32_t>& chunkIndices, std::array<bool, 6>& facesToDraw, uint32_t startIndex) {
    const std::array<uint32_t, 6> newIndices[6] = {
        { startIndex + 0, startIndex + 2, startIndex + 1, startIndex + 3, startIndex + 1, startIndex + 2 }, // top
        { startIndex + 7, startIndex + 6, startIndex + 5, startIndex + 4, startIndex + 5, startIndex + 6 }, // bottom
        { startIndex + 1, startIndex + 5, startIndex + 0, startIndex + 4, startIndex + 0, startIndex + 5 }, // front
        { startIndex + 3, startIndex + 2, startIndex + 7, startIndex + 6, startIndex + 7, startIndex + 2 }, // back
        { startIndex + 7, startIndex + 5, startIndex + 3, startIndex + 1, startIndex + 3, startIndex + 5 }, // left
        { startIndex + 0, startIndex + 4, startIndex + 2, startIndex + 6, startIndex + 2, startIndex + 4 }  // right
    };

    for (size_t i = 0; i < 6; i++) {
        if (facesToDraw[i]) {
            chunkIndices.insert(chunkIndices.end(), newIndices[i].begin(), newIndices[i].end());
        }
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

    return{
        {{startPos.x + left, startPos.y - bottom},
            {1.0f, 0.0f, 0.0f},
            {texLeft, texTop}},

        {{startPos.x + right, startPos.y - bottom},
            {0.0f, 1.0f, 0.0f},
            {texRight, texTop}},

        {{startPos.x + right, startPos.y - top},
            {0.0f, 0.0f, 1.0f},
            {texRight, texBottom}},

        {{startPos.x + left, startPos.y - top},
            {1.0f, 1.0f, 1.0f},
            {texLeft, texBottom}}
    };
}

std::vector<uint32_t> generateTexturedQuadIndices(uint32_t startIndex) {
    return {
        startIndex + 0, startIndex + 1, startIndex + 2,
        startIndex + 3, startIndex + 0, startIndex + 2
    };;
}
