#include "VulkanBufferUtil.h"

#include <stdexcept>

#include "CoreRenderer.h"
#include "VulkanUtil.h"

// OBJECT CREATION FUNCTIONS
void createBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties) {
    destroyBuffer(buffer, bufferMemory);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(CoreRenderer::device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(CoreRenderer::device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
            findMemoryType(CoreRenderer::physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(CoreRenderer::device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(CoreRenderer::device, buffer, bufferMemory, 0);
}

void destroyBuffer(const VkBuffer &buffer, const VkDeviceMemory &bufferMemory) {
    if (buffer != nullptr) {
        vkDeviceWaitIdle(CoreRenderer::device);
        vkFreeMemory(CoreRenderer::device, bufferMemory, nullptr);
        vkDestroyBuffer(CoreRenderer::device, buffer, nullptr);
    }
}

template void createVertexBuffer<TexturedVertex>(
    VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory, VkDeviceSize bufferSize,
    const std::vector<TexturedVertex> &vertices);

template void createVertexBuffer<ChunkVertex>(
    VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory, VkDeviceSize bufferSize,
    const std::vector<ChunkVertex> &vertices);

template<typename VertexType>
void createVertexBuffer(VkBuffer &vertexBuffer, VkDeviceMemory &vertexBufferMemory, VkDeviceSize bufferSize,
                        const std::vector<VertexType> &vertices) {
    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    createBuffer(stagingBuffer, stagingBufferMemory, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(CoreRenderer::device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(CoreRenderer::device, stagingBufferMemory);

    createBuffer(vertexBuffer, vertexBufferMemory, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    destroyBuffer(stagingBuffer, stagingBufferMemory);
}

void createIndexBuffer(VkBuffer &indexBuffer, VkDeviceMemory &indexBufferMemory, VkDeviceSize bufferSize,
                       const std::vector<uint32_t> &indices) {
    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    createBuffer(stagingBuffer, stagingBufferMemory, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data;
    vkMapMemory(CoreRenderer::device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(CoreRenderer::device, stagingBufferMemory);

    createBuffer(indexBuffer, indexBufferMemory, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);
    destroyBuffer(stagingBuffer, stagingBufferMemory);
}

void createUniformBuffers(std::vector<VkBuffer> &uniformBuffers,
                          std::vector<VkDeviceMemory> &uniformBuffersMemory,
                          std::vector<void *> &uniformBuffersMapped) {
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        createBuffer(uniformBuffers[i], uniformBuffersMemory[i], bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(CoreRenderer::device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

//GENERAL UTILITY FUNCTIONS
void copyBuffer(VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void copyBufferRanges(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, uint32_t objectSize,
                      std::unordered_map<uint32_t, ChunkMemoryRange> &memoryRanges) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    std::vector<VkBufferCopy> rangesToCopy;

    for (auto &[chunkID, memoryRange]: memoryRanges) {
        if (!memoryRange.savedToVBuffer) {
            VkBufferCopy copyRegion{};
            uint32_t startByte = memoryRange.startPos * objectSize;
            copyRegion.srcOffset = startByte;
            copyRegion.dstOffset = startByte;
            copyRegion.size = memoryRange.objectCount * objectSize;
            memoryRange.savedToVBuffer = true;
            rangesToCopy.push_back(copyRegion);
            break;
        }
    }

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, rangesToCopy.size(), rangesToCopy.data());

    endSingleTimeCommands(commandBuffer);
}

void copyBufferToImage(const VkBuffer &buffer, const VkImage &image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void updateBuffer(const VkBuffer &buffer, const VkBuffer &stagingBuffer, const VkDeviceMemory &stagingBufferMemory,
                  void *newData, VkDeviceSize bufferSize, uint32_t objectSize,
                  std::unordered_map<uint32_t, ChunkMemoryRange> &memoryRanges) {
    void *data;
    vkMapMemory(CoreRenderer::device, stagingBufferMemory, 0, bufferSize, 0, &data);

    bool regionUpdateFound = false;
    for (auto &[chunkID, memoryRange]: memoryRanges) {
        if (!memoryRange.savedToVBuffer) {
            const uint32_t memoryOffset = memoryRange.startPos * objectSize;
            memcpy(static_cast<char *>(data) + memoryOffset,
                   static_cast<char *>(newData) + memoryOffset,
                   memoryRange.objectCount * objectSize);
            regionUpdateFound = true;
        }
    }

    vkUnmapMemory(CoreRenderer::device, stagingBufferMemory);

    if (!regionUpdateFound) {
        return;
    }

    copyBufferRanges(stagingBuffer, buffer, objectSize, memoryRanges);
}

void updateDrawParamsBuffer(const VkDeviceMemory &bufferMemory, VkDeviceSize bufferSize) {
    if (bufferSize == 0) {
        return;
    }

    void *data;
    vkMapMemory(CoreRenderer::device, bufferMemory, 0, bufferSize, 0, &data);

    uint32_t commandIndex = 0;
    for (auto &[chunkID, memoryRange]: VertexPool::getOccupiedIndexRanges()) {
        VkDrawIndexedIndirectCommand command;
        command.indexCount = memoryRange.objectCount;
        command.instanceCount = 1;
        command.firstIndex = memoryRange.startPos;
        command.vertexOffset = static_cast<int32_t>(memoryRange.offset);
        command.firstInstance = 0;

        memcpy(static_cast<char *>(data) + commandIndex * sizeof(VkDrawIndexedIndirectCommand),
               &command,
               sizeof(VkDrawIndexedIndirectCommand));

        commandIndex++;
    }

    vkUnmapMemory(CoreRenderer::device, bufferMemory);
}