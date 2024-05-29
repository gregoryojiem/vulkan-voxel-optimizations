#ifndef VERTEX_H
#define VERTEX_H

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include <array>

#include "../game/Block.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

extern std::vector<Vertex> globalChunkVertices;
extern std::vector<uint32_t> globalChunkIndices;

extern void addVerticesAndIndices(const std::vector<Vertex>& newVertices, const std::vector<uint32_t>& newIndices);

#endif //VERTEX_H
