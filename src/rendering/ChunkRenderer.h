#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <vector>

#include "SwapChain.h"
#include "VulkanDebugger.h"

class GameRenderer {
public:
    void init();

    void draw(const VkCommandBuffer &commandBuffer, uint32_t currentFrame);

    void cleanup(const VkDevice &device, uint32_t maxFramesInFlight) const;

private:
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};

    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
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

#endif //GAMERENDERER_H
