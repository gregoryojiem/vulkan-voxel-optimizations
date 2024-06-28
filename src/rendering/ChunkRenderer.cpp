#include "ChunkRenderer.h"


#include <cstring>
#include <cstdint>
#include <fstream>

#include "../core/Chunk.h"
#include "vulkan/VulkanBufferUtil.h"
#include "vulkan/VulkanUtil.h"
#include "scene/VertexPool.h"

constexpr uint32_t INDEX_BUFFER_SIZE = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 36;

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
    createVertexBuffer(vertexBuffer, vertexBufferMemory, vertexMemorySize, globalChunkVertices);
    std::vector<uint32_t> indices(INDEX_BUFFER_SIZE);
    fillChunkIndices(indices);
    createIndexBuffer(indexBuffer, indexBufferMemory, indices.size(), indices);
    createStagingBuffer(vertexStagingBuffer, vertexStagingBufferMemory, vertexMemorySize);
}

void ChunkRenderer::draw(const VkCommandBuffer &commandBuffer, uint32_t currentFrame, const UniformBufferObject &ubo) {
    resizeBuffers();
    updateBuffers(ubo);
    if (VertexPool::getOccupiedVertexRanges().empty()) {
        return;
    }
    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    const VkBuffer vertexBuffers[] = {vertexBuffer};
    constexpr VkDeviceSize offsets[] = {0};

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &descriptorSets[currentFrame], 0, nullptr);

    const uint32_t drawCount = VertexPool::getOccupiedVertexRanges().size() * VISIBLE_BLOCK_SIDES;
    vkCmdDrawIndexedIndirect(commandBuffer, drawParamsBuffer, 0, drawCount, sizeof(VkDrawIndexedIndirectCommand));
}

void ChunkRenderer::fillChunkIndices(std::vector<uint32_t> &indices) {
    int indexStartPos = 0;
    for (uint32_t i = 0; i < indices.size(); i += 6) {
        indices.at(i) = indexStartPos;
        indices.at(i + 1) = indexStartPos + 2;
        indices.at(i + 2) = indexStartPos + 1;
        indices.at(i + 3) = indexStartPos + 3;
        indices.at(i + 4) = indexStartPos + 1;
        indices.at(i + 5) = indexStartPos + 2;
        indexStartPos += 4;
    }
}

void ChunkRenderer::resizeBuffers() {
    uint32_t verticesSize = sizeof(globalChunkVertices[0]) * globalChunkVertices.size();
    uint32_t drawParamsSize = sizeof(VkDrawIndexedIndirectCommand) * VertexPool::getOccupiedVertexRanges().size() *
                              VISIBLE_BLOCK_SIDES;

    //todo benchmark to see if fixed size buffers here would be better for high update counts
    if (verticesSize > vertexMemorySize) {
        vertexMemorySize = verticesSize;
        createVertexBuffer(vertexBuffer, vertexBufferMemory, vertexMemorySize, globalChunkVertices);
        createStagingBuffer(vertexStagingBuffer, vertexStagingBufferMemory, vertexMemorySize);
    }
    if (drawParamsSize > drawParamsMemorySize) {
        drawParamsMemorySize = drawParamsSize;
        createIndirectBuffer(drawParamsBuffer, drawParamsBufferMemory, drawParamsSize);
    }
}

void ChunkRenderer::updateBuffers(const UniformBufferObject &ubo) const {
    bool facesToRender[6] = {true, true, true, true, true, true};
    updateDrawParamsBuffer(drawParamsBufferMemory, VertexPool::getOccupiedVertexRanges().size(), facesToRender);
    if (!VertexPool::newUpdate) {
        return;
    }
    updateChunkBuffer(vertexBuffer, vertexStagingBuffer, vertexStagingBufferMemory, globalChunkVertices.data(),
                      vertexMemorySize, sizeof(ChunkVertex), VertexPool::getOccupiedVertexRanges());
    VertexPool::newUpdate = false;
}

void ChunkRenderer::cleanup(const VkDevice &device, uint32_t maxFramesInFlight) const {
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        destroyBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
    }

    destroyBuffer(indexBuffer, indexBufferMemory);
    destroyBuffer(vertexBuffer, vertexBufferMemory);
    destroyBuffer(vertexStagingBuffer, vertexStagingBufferMemory);
    destroyBuffer(drawParamsBuffer, drawParamsBufferMemory);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
