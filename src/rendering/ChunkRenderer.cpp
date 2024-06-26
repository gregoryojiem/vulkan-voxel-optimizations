#include "ChunkRenderer.h"


#include <iostream>
#include <cstdint>
#include <fstream>

#include "vulkan/VulkanBufferUtil.h"
#include "vulkan/VulkanUtil.h"
#include "scene/VertexPool.h"

void ChunkRenderer::init(VkDescriptorPool &descriptorPool, VkRenderPass &renderPass) {
    createUniformBuffers(uniformBuffers, uniformBuffersMemory, uniformBuffersMapped);
    createDescriptorSetLayout(descriptorSetLayout, true, false);
    createUBDescriptorSets(descriptorSets, descriptorSetLayout, descriptorPool, uniformBuffers);
    createGraphicsPipeline(
        pipelineLayout, graphicsPipeline, descriptorSetLayout, renderPass,
        "../src/rendering/shaders/vert.spv",
        "../src/rendering/shaders/frag.spv",
        ChunkVertex::getBindingDescription(),
        ChunkVertex::getAttributeDescriptions(),
        true, true);
    vertexMemorySize = sizeof(globalChunkVertices[0]) * globalChunkVertices.size();
    indexMemorySize = sizeof(globalChunkIndices[0]) * globalChunkIndices.size();
    createVertexBuffer(vertexBuffer, vertexBufferMemory, vertexMemorySize, globalChunkVertices);
    createIndexBuffer(indexBuffer, indexBufferMemory, indexMemorySize, globalChunkIndices);
    createStagingBuffer(vertexStagingBuffer, vertexStagingBufferMemory, vertexMemorySize);
    createStagingBuffer(indexStagingBuffer, indexStagingBufferMemory, indexMemorySize);
}

void ChunkRenderer::draw(const VkCommandBuffer &commandBuffer, uint32_t currentFrame, const UniformBufferObject &ubo) {
    resizeBuffers();
    updateBuffers();

    const VkBuffer vertexBuffers[] = {vertexBuffer};
    constexpr VkDeviceSize offsets[] = {0};

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &descriptorSets[currentFrame], 0, nullptr);
    const uint32_t drawCount = VertexPool::getOccupiedIndexRanges().size();
    vkCmdDrawIndexedIndirect(commandBuffer, drawParamsBuffer, 0, drawCount, sizeof(VkDrawIndexedIndirectCommand));

    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void ChunkRenderer::resizeBuffers() {
    uint32_t verticesSize = sizeof(globalChunkVertices[0]) * globalChunkVertices.size();
    uint32_t indicesSize = sizeof(globalChunkIndices[0]) * globalChunkIndices.size();
    uint32_t drawParamsSize = sizeof(VkDrawIndexedIndirectCommand) * VertexPool::getOccupiedIndexRanges().size();

    //todo benchmark with fixed size buffers here
    if (verticesSize > vertexMemorySize) {
        vertexMemorySize = verticesSize;
        createVertexBuffer(vertexBuffer, vertexBufferMemory, vertexMemorySize, globalChunkVertices);
        createStagingBuffer(vertexStagingBuffer, vertexStagingBufferMemory, vertexMemorySize);
    }
    if (indicesSize > indexMemorySize) {
        indexMemorySize = indicesSize;
        createIndexBuffer(indexBuffer, indexBufferMemory, indexMemorySize, globalChunkIndices);
        createStagingBuffer(indexStagingBuffer, indexStagingBufferMemory, indexMemorySize);
    }
    if (drawParamsSize > drawParamsMemorySize) {
        drawParamsMemorySize = drawParamsSize;
        createIndirectBuffer(drawParamsBuffer, drawParamsBufferMemory, drawParamsSize);
    }
}

void ChunkRenderer::updateBuffers() const {
    if (!VertexPool::newUpdate) {
        return;
    }
    updateChunkBuffer(vertexBuffer, vertexStagingBuffer, vertexStagingBufferMemory, globalChunkVertices.data(),
                      vertexMemorySize, sizeof(ChunkVertex), VertexPool::getOccupiedVertexRanges());
    updateChunkBuffer(indexBuffer, indexStagingBuffer, indexStagingBufferMemory, globalChunkIndices.data(),
                      indexMemorySize, sizeof(globalChunkIndices[0]), VertexPool::getOccupiedIndexRanges());
    updateDrawParamsBuffer(drawParamsBufferMemory, VertexPool::getOccupiedIndexRanges().size());
    VertexPool::newUpdate = false;
}

void ChunkRenderer::cleanup(const VkDevice &device, uint32_t maxFramesInFlight) const {
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
    }

    destroyBuffer(indexBuffer, indexBufferMemory);
    destroyBuffer(indexStagingBuffer, indexStagingBufferMemory);
    destroyBuffer(vertexBuffer, vertexBufferMemory);
    destroyBuffer(vertexStagingBuffer, vertexStagingBufferMemory);
    destroyBuffer(drawParamsBuffer, drawParamsBufferMemory);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
