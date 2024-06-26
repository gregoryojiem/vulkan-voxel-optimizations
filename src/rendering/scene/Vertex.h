#ifndef VERTEX_H
#define VERTEX_H

#include <vector>
#include <../../../dependencies/glm-1.0.1/glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct ChunkVertex {
    glm::vec3 position;
    uint8_t color[4];

    static VkVertexInputBindingDescription getBindingDescription();

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

struct TexturedVertex {
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

#endif //VERTEX_H
