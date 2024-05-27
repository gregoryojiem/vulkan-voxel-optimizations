#include "WorldGeometry.h"

std::vector<Vertex> vertices = { };

std::vector<uint16_t> indices = { };

void addVerticesAndIndices(const std::vector<Vertex>& newVertices, const std::vector<uint16_t>& newIndices) {
    vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
    indices.insert(indices.end(), newIndices.begin(), newIndices.end());
}

void tesselateAndAddBlock(Block block) {
    std::vector<Vertex> const vertices = {
        {{0.5f + block.position.x, 0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, 0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{0.5f + block.position.x, 0.5f + block.position.y, -0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, 0.5f + block.position.y, -0.5f + block.position.z}, block.color},

        {{0.5f + block.position.x, -0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, -0.5f + block.position.y, 0.5f + block.position.z}, block.color},
        {{0.5f + block.position.x, -0.5f + block.position.y, -0.5f + block.position.z}, block.color},
        {{-0.5f + block.position.x, -0.5f + block.position.y, -0.5f + block.position.z}, block.color},
    };

    std::vector<uint16_t> const indices = {
        //top face (facing (0, -1, 0))
        0, 2, 1,
        3, 1, 2,

        //bottom face
        7, 6, 5,
        4, 5, 6,

        //front face (facing (0, 0, -1))
        1, 5, 0,
        4, 0, 5,

        //back face
        3, 2, 7,
        6, 7, 2,

        //left face (facing (-1, 0, 0))
        7, 5, 3,
        1, 3, 5,

        //right face
        0, 4, 2,
        6, 2, 4
    };

    addVerticesAndIndices(vertices, indices);
}

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}