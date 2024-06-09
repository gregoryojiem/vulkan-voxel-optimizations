#ifndef VULKANBUFFERUTIL_H
#define VULKANBUFFERUTIL_H

#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VertexPool.h"

// OBJECT CREATION FUNCTIONS
extern void createBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize size, const VkDevice& device,
    const VkPhysicalDevice& physDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

extern void destroyBuffer(const VkBuffer& buffer, const VkDeviceMemory& bufferMemory, const VkDevice& device);

template <typename VertexType>
extern void createVertexBuffer(VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, VkDeviceSize bufferSize,
const std::vector<VertexType>& vertices, const VkDevice& device, const VkPhysicalDevice& physDevice,
const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

extern void createIndexBuffer(VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, VkDeviceSize bufferSize,
    const std::vector<uint32_t>& indices, const VkDevice& device, const VkPhysicalDevice& physDevice,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

extern void createUniformBuffers(std::vector<VkBuffer>& uniformBuffers,
    std::vector<VkDeviceMemory>& uniformBuffersMemory, std::vector<void*>& uniformBuffersMapped,
    uint32_t maxFramesInFlight, const VkDevice& device, const VkPhysicalDevice& physDevice);

//GENERAL UTILITY FUNCTIONS
extern void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, const VkDevice& device,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

extern void copyBufferRanges(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer,uint32_t objectSize,
    std::unordered_map<uint32_t, ChunkMemoryRange>& memoryRanges, const VkDevice& device,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

extern void updateBuffer(const VkBuffer& buffer, const VkBuffer& stagingBuffer, const VkDeviceMemory& stagingBufferMemory,
    void* newData, VkDeviceSize bufferSize, uint32_t objectSize, const VkDevice& device,
    std::unordered_map<uint32_t, ChunkMemoryRange>& memoryRanges, const VkCommandPool& commandPool,
    const VkQueue& graphicsQueue);

extern void copyBufferToImage(const VkBuffer& buffer, const VkImage& image, uint32_t width, uint32_t height,
    const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

#endif //VULKANBUFFERUTIL_H
