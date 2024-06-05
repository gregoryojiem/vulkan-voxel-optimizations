#include "GraphicsUtil.h"

#include "../rendering/Vertex.h"

std::vector<ChunkVertex> generateBlockVertices(Block block) {
    return {
        {{0.5f + block.position.x, 0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, 0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{0.5f + block.position.x, 0.5f + block.position.y, -0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, 0.5f + block.position.y, -0.5f + block.position.z}, block.color},

        {{0.5f + block.position.x, -0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, -0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{0.5f + block.position.x, -0.5f + block.position.y, -0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, -0.5f + block.position.y, -0.5f + block.position.z}, block.color},
    };
}

std::vector<uint32_t> generateBlockIndices(uint32_t startIndex) {
    return {
        //top face (facing (0, -1, 0))
        startIndex + 0, startIndex + 2, startIndex + 1,
        startIndex + 3, startIndex + 1, startIndex + 2,

        //bottom face
        startIndex + 7, startIndex + 6, startIndex + 5,
        startIndex + 4, startIndex + 5, startIndex + 6,

        //front face (facing (0, 0, -1))
        startIndex + 1, startIndex + 5, startIndex + 0,
        startIndex + 4, startIndex + 0, startIndex + 5,

        //back face
        startIndex + 3, startIndex + 2, startIndex + 7,
        startIndex + 6, startIndex + 7, startIndex + 2,

        //left face (facing (-1, 0, 0))
        startIndex + 7, startIndex + 5, startIndex + 3,
        startIndex + 1, startIndex + 3, startIndex + 5,

        //right face
        startIndex + 0, startIndex + 4, startIndex + 2,
        startIndex + 6, startIndex + 2, startIndex + 4
    };
}

std::vector<TexturedVertex> generateTexturedQuad(glm::vec4 quadBounds, glm::vec4 texQuadBounds,
    glm::vec2 startPos, float scale) {

    float left = quadBounds[0];
    float bottom = quadBounds[1];
    float right = quadBounds[2];
    float top = quadBounds[3];

    // top and bottom switched due to y format differences
    float texLeft = texQuadBounds[0];
    float texTop = texQuadBounds[1];
    float texRight = texQuadBounds[2];
    float texBottom = texQuadBounds[3];

    float adjScale = scale;

    return{
        {{startPos.x + left * adjScale, startPos.y + bottom * adjScale},
            {1.0f, 0.0f, 0.0f},
            {texLeft, texTop}},

        {{startPos.x + right * adjScale, startPos.y + bottom * adjScale},
            {0.0f, 1.0f, 0.0f},
            {texRight, texTop}},

        {{startPos.x + right * adjScale, startPos.y + top * adjScale},
            {0.0f, 0.0f, 1.0f},
            {texRight, texBottom}},

        {{startPos.x + left * adjScale, startPos.y + top * adjScale},
            {1.0f, 1.0f, 1.0f},
            {texLeft, texBottom}}
    };
}

std::vector<uint32_t> generateTexturedQuadIndices(uint32_t startIndex) {
    return {
        startIndex + 0, startIndex + 1, startIndex + 2,
        startIndex + 2, startIndex + 3, startIndex + 0
    };;
}
