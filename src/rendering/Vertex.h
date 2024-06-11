#ifndef VERTEX_H
#define VERTEX_H

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct ChunkVertex {
    glm::vec3 pos;
    uint8_t color[4];

    static VkVertexInputBindingDescription getBindingDescription();

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

struct TexturedVertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

#endif //VERTEX_H
