#include "VulkanBufferUtil.h"

#include <stdexcept>
#include <string.h>

#include "VulkanUtil.h"

// OBJECT CREATION FUNCTIONS
void createBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize size, const VkDevice& device,
    const VkPhysicalDevice& physDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void destroyBuffer(const VkBuffer& buffer, const VkDeviceMemory& bufferMemory, const VkDevice& device) {
    if (buffer != nullptr) {
        vkDeviceWaitIdle(device);
        vkFreeMemory(device, bufferMemory, nullptr);
        vkDestroyBuffer(device, buffer, nullptr);
    }
}

template void createVertexBuffer<TexturedVertex>(
    VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory,VkDeviceSize bufferSize,
    const std::vector<TexturedVertex>& vertices, const VkDevice& device, const VkPhysicalDevice& physDevice,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

template void createVertexBuffer<ChunkVertex>(
    VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, VkDeviceSize bufferSize,
    const std::vector<ChunkVertex>& vertices, const VkDevice& device, const VkPhysicalDevice& physDevice,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue);

template <typename VertexType>
void createVertexBuffer(VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, VkDeviceSize bufferSize,
const std::vector<VertexType>& vertices, const VkDevice& device, const VkPhysicalDevice& physDevice,
const VkCommandPool& commandPool, const VkQueue& graphicsQueue) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(stagingBuffer, stagingBufferMemory, bufferSize, device, physDevice,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(vertexBuffer, vertexBufferMemory, bufferSize, device, physDevice,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize, device, commandPool, graphicsQueue);
    destroyBuffer(stagingBuffer, stagingBufferMemory, device);
}

void createIndexBuffer(VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, VkDeviceSize bufferSize,
    const std::vector<uint32_t>& indices, const VkDevice& device, const VkPhysicalDevice& physDevice,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(stagingBuffer, stagingBufferMemory, bufferSize, device, physDevice,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(indexBuffer, indexBufferMemory, bufferSize, device, physDevice,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize, device, commandPool, graphicsQueue);
    destroyBuffer(stagingBuffer, stagingBufferMemory, device);
}

void createUniformBuffers(std::vector<VkBuffer>& uniformBuffers,
    std::vector<VkDeviceMemory>& uniformBuffersMemory, std::vector<void*>& uniformBuffersMapped,
    uint32_t maxFramesInFlight, const VkDevice& device, const VkPhysicalDevice& physDevice) {
    uniformBuffers.resize(maxFramesInFlight);
    uniformBuffersMemory.resize(maxFramesInFlight);
    uniformBuffersMapped.resize(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        createBuffer(uniformBuffers[i], uniformBuffersMemory[i], bufferSize,
            device, physDevice,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

//GENERAL UTILITY FUNCTIONS
void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, const VkDevice& device,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(device, commandPool, commandBuffer, graphicsQueue);
}

void copyBufferRanges(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, uint32_t objectSize,
    std::unordered_map<uint32_t, ChunkMemoryRange>& memoryRanges, const VkDevice& device,
    const VkCommandPool& commandPool, const VkQueue& graphicsQueue) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    std::vector<VkBufferCopy> rangesToCopy;

    for (auto& [chunkID, memoryRange] : memoryRanges) {
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

    endSingleTimeCommands(device, commandPool, commandBuffer, graphicsQueue);
}

void updateBuffer(const VkBuffer& buffer, const VkBuffer& stagingBuffer, const VkDeviceMemory& stagingBufferMemory,
    void* newData, VkDeviceSize bufferSize, uint32_t objectSize, const VkDevice& device,
    std::unordered_map<uint32_t, ChunkMemoryRange>& memoryRanges, const VkCommandPool& commandPool,
    const VkQueue& graphicsQueue) {
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);

    bool regionUpdateFound = false;
    for (auto& [chunkID, memoryRange] : memoryRanges) {
        if (!memoryRange.savedToVBuffer) {
            const uint32_t memoryOffset = memoryRange.startPos * objectSize;
            memcpy(static_cast<char*>(data) + memoryOffset,
                static_cast<char*>(newData) + memoryOffset,
                memoryRange.objectCount * objectSize);
            regionUpdateFound = true;
        }
    }

    vkUnmapMemory(device, stagingBufferMemory);

    if (!regionUpdateFound) {
        return;
    }

    copyBufferRanges(stagingBuffer, buffer, objectSize, memoryRanges, device, commandPool, graphicsQueue);
}

void copyBufferToImage(const VkBuffer& buffer, const VkImage& image, uint32_t width, uint32_t height,
    const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& graphicsQueue) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

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

    endSingleTimeCommands(device, commandPool, commandBuffer, graphicsQueue);
}
