#ifndef VULKANBUFFERUTIL_H
#define VULKANBUFFERUTIL_H

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VertexPool.h"

// OBJECT CREATION FUNCTIONS
static void createBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize size,
                         VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

extern void destroyBuffer(const VkBuffer &buffer, const VkDeviceMemory &bufferMemory);

template<typename VertexType>
extern void createVertexBuffer(VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory, VkDeviceSize bufferSize,
                               const std::vector<VertexType> &vertices);

extern void createIndexBuffer(VkBuffer &indexBuffer, VkDeviceMemory &indexBufferMemory, VkDeviceSize bufferSize,
                              const std::vector<uint32_t> &indices);

extern void createStagingBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize bufferSize);

extern void createIndirectBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize bufferSize);

extern void createUniformBuffers(std::vector<VkBuffer> &uniformBuffers,
                                 std::vector<VkDeviceMemory> &uniformBuffersMemory,
                                 std::vector<void *> &uniformBuffersMapped);

extern void createShaderImageFromFile(VkImage &image, VkDeviceMemory &imageMemory, int &width, int &height,
                                      const std::string &path);

//GENERAL UTILITY FUNCTIONS
extern void copyBuffer(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, VkDeviceSize size);

extern void copyBufferRanges(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, uint32_t objectSize,
                             std::unordered_map<uint32_t, ChunkMemoryRange> &memoryRanges);

extern void copyBufferToImage(const VkBuffer &buffer, const VkImage &image, uint32_t width, uint32_t height);

extern void updateBuffer(const VkBuffer &buffer, const VkBuffer &stagingBuffer,
                         const VkDeviceMemory &stagingBufferMemory, const void *newData, VkDeviceSize bufferSize);

extern void updateChunkBuffer(const VkBuffer &buffer, const VkBuffer &stagingBuffer,
                              const VkDeviceMemory &stagingBufferMemory,
                              void *newData, VkDeviceSize bufferSize, uint32_t objectSize,
                              std::unordered_map<uint32_t, ChunkMemoryRange> &memoryRanges);

extern void updateDrawParamsBuffer(const VkDeviceMemory &bufferMemory, VkDeviceSize bufferSize);

#endif //VULKANBUFFERUTIL_H
