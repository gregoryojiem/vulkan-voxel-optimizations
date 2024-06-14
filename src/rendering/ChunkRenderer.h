#ifndef CHUNKRENDERER_H
#define CHUNKRENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <vector>

#include "vulkan/VulkanStructs.h"

class ChunkRenderer {
public:
    void init(VkDescriptorPool &descriptorPool);

    void draw(const VkCommandBuffer &commandBuffer, uint32_t currentFrame, const UniformBufferObject &ubo);

    void cleanup(const VkDevice &device, uint32_t maxFramesInFlight) const;

private:
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};

    VkDescriptorSetLayout descriptorSetLayout{};
    std::vector<VkDescriptorSet> descriptorSets;

    VkBuffer vertexBuffer{};
    VkDeviceMemory vertexBufferMemory{};
    uint32_t vertexMemorySize{};
    VkBuffer vertexStagingBuffer{};
    VkDeviceMemory vertexStagingBufferMemory{};

    VkBuffer indexBuffer{};
    VkDeviceMemory indexBufferMemory{};
    uint32_t indexMemorySize{};
    VkBuffer indexStagingBuffer{};
    VkDeviceMemory indexStagingBufferMemory{};

    VkBuffer drawParamsBuffer{};
    VkDeviceMemory drawParamsBufferMemory{};
    uint32_t drawParamsMemorySize{};

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;

    void resizeBuffers();

    void updateBuffers() const;
};

#endif //CHUNKRENDERER_H
