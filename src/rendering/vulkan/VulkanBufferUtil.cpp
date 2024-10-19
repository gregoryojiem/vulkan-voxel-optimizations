#include "VulkanBufferUtil.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

#include "../CoreRenderer.h"
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
            findMemoryType(memRequirements.memoryTypeBits, properties);

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

void createStagingBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize bufferSize) {
    createBuffer(buffer, bufferMemory, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void createIndirectBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize bufferSize) {
    createBuffer(buffer, bufferMemory, bufferSize,
                 VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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

void createShaderImage(VkImage &image, VkDeviceMemory &imageMemory, void *newData, int width, int height) {
    uint32_t imageSize = width * height * 4;

    createImage(image, imageMemory, width, height,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    createStagingBuffer(stagingBuffer, stagingBufferMemory, imageSize);

    void *data;
    vkMapMemory(CoreRenderer::device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, newData, imageSize);
    vkUnmapMemory(CoreRenderer::device, stagingBufferMemory);

    transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, image, width, height);
    transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    destroyBuffer(stagingBuffer, stagingBufferMemory);
}

void createShaderImageFromFile(VkImage &image, VkDeviceMemory &imageMemory, int &width, int &height,
                               const std::string &path) {
    int channels;
    stbi_uc *pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (width == 0 || height == 0) {
        throw std::runtime_error("failed to open shader image!");
    }

    createShaderImage(image, imageMemory, pixels, width, height);
}

//GENERAL UTILITY FUNCTIONS
void copyBuffer(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void copyBufferRanges(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, uint32_t objectSize,
                      std::unordered_map<glm::vec3, ChunkMemoryRange> &memoryRanges) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    std::vector<VkBufferCopy> rangesToCopy;
    for (auto &[chunkID, memoryRange]: memoryRanges) {
        if (!memoryRange.savedToVBuffer) {
            VkBufferCopy copyRegion{};
            uint32_t startByte = memoryRange.startPos * objectSize;
            copyRegion.srcOffset = startByte;
            copyRegion.dstOffset = startByte;
            copyRegion.size = memoryRange.vertexCount * objectSize;
            memoryRange.savedToVBuffer = true;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        }
    }
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
                  const void *newData, VkDeviceSize bufferSize) {
    void *data;
    vkMapMemory(CoreRenderer::device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, newData, bufferSize);
    vkUnmapMemory(CoreRenderer::device, stagingBufferMemory);
    copyBuffer(stagingBuffer, buffer, bufferSize);
}

void updateChunkBuffer(const VkBuffer &buffer, const VkBuffer &stagingBuffer, const VkDeviceMemory &stagingBufferMemory,
                       void *newData, VkDeviceSize bufferSize, uint32_t objectSize,
                       std::unordered_map<glm::vec3, ChunkMemoryRange> &memoryRanges) {
    void *data;
    vkMapMemory(CoreRenderer::device, stagingBufferMemory, 0, bufferSize, 0, &data);

    bool regionUpdateFound = false;
    for (auto &[chunkID, memoryRange]: memoryRanges) {
        if (!memoryRange.savedToVBuffer) {
            const uint32_t memoryOffset = memoryRange.startPos * objectSize;
            memcpy(static_cast<char *>(data) + memoryOffset,
                   static_cast<char *>(newData) + memoryOffset,
                   memoryRange.vertexCount * objectSize);
            regionUpdateFound = true;
        }
    }

    vkUnmapMemory(CoreRenderer::device, stagingBufferMemory);
    if (!regionUpdateFound) {
        return;
    }
    copyBufferRanges(stagingBuffer, buffer, objectSize, memoryRanges);
}

void updateDrawParamsBuffer(const VkDeviceMemory &bufferMemory, VkDeviceSize bufferSize, const Frustrum &frustrum, const glm::vec3 &frontVec) {
    if (bufferSize == 0) {
        return;
    }

    void *data;
    vkMapMemory(CoreRenderer::device, bufferMemory, 0, bufferSize, 0, &data);
    uint32_t commandIndex = 0;
    for (auto &[chunkID, memoryRange]: VertexPool::getOccupiedVertexRanges()) {
        for (int face = 0; face < 6; face++) {
            if (face == 0 && frontVec.y < 0) {
                continue;
            }
            if (face == 1 && frontVec.y > 0) {
                continue;
            }
            VkDrawIndexedIndirectCommand command;
            command.indexCount = VertexPool::getIndexCount(memoryRange, face);
            command.instanceCount = 1;
            command.firstIndex = 0;
            command.vertexOffset = static_cast<int32_t>(memoryRange.startPos + memoryRange.faceOffsets[face]);
            command.firstInstance = 0;
            memcpy(static_cast<char *>(data) + commandIndex * sizeof(VkDrawIndexedIndirectCommand), &command,
                   sizeof(VkDrawIndexedIndirectCommand));
            commandIndex++;
        }
    }

    vkUnmapMemory(CoreRenderer::device, bufferMemory);
}
