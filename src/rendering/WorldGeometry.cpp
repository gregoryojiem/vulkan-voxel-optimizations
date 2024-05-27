#include "WorldGeometry.h"

std::vector<Vertex> vertices = { };

std::vector<uint32_t> indices = { };

void addVerticesAndIndices(const std::vector<Vertex>& newVertices, const std::vector<uint32_t>& newIndices) {
    vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
    indices.insert(indices.end(), newIndices.begin(), newIndices.end());
}

void tesselateAndAddBlock(Block block) {
    uint16_t newIndex = static_cast<uint16_t>(vertices.size());
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



    std::vector const indices = {
        //top face (facing (0, -1, 0))
        newIndex + 0u, newIndex + 2u, newIndex + 1u,
        newIndex + 3u, newIndex + 1u, newIndex + 2u,

        //bottom face
        newIndex + 7u, newIndex + 6u, newIndex + 5u,
        newIndex + 4u, newIndex + 5u, newIndex + 6u,

        //front face (facing (0, 0, -1))
        newIndex + 1u, newIndex + 5u, newIndex + 0u,
        newIndex + 4u, newIndex + 0u, newIndex + 5u,

        //back face
        newIndex + 3u, newIndex + 2u, newIndex + 7u,
        newIndex + 6u, newIndex + 7u, newIndex + 2u,

        //left face (facing (-1, 0, 0))
        newIndex + 7u, newIndex + 5u, newIndex + 3u,
        newIndex + 1u, newIndex + 3u, newIndex + 5u,

        //right face
        newIndex + 0u, newIndex + 4u, newIndex + 2u,
        newIndex + 6u, newIndex + 2u, newIndex + 4u
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