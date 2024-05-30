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

#endif //VERTEX_H
