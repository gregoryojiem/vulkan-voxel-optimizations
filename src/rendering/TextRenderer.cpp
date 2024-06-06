#include "TextRenderer.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "GameRenderer.h"
#include "../utility/GraphicsUtil.h"

int TextRenderer::atlasWidth;
int TextRenderer::atlasHeight;

std::vector<TexturedVertex> TextRenderer::textQuadVertices;
std::vector<uint32_t> TextRenderer::textQuadIndices;

std::vector<ScreenText> TextRenderer::activeText;
std::map<char, Character> TextRenderer::characters;
const std::string TextRenderer::fontPath = "resources/fonts";
const std::string TextRenderer::fontToUse = "abel";

VkBuffer TextRenderer::textVertexBuffer;
VkDeviceMemory TextRenderer::textVertexBufferMemory;
uint32_t TextRenderer::textVertexMemorySize;
VkBuffer TextRenderer::textStagingBuffer;
VkDeviceMemory TextRenderer::textStagingBufferMemory;

VkBuffer TextRenderer::textIndexBuffer;
VkDeviceMemory TextRenderer::textIndexBufferMemory;

std::vector<VkBuffer> TextRenderer::textUniformBuffers;
std::vector<VkDeviceMemory> TextRenderer::textUniformBuffersMemory;
std::vector<void*> TextRenderer::textUniformBuffersMapped;

VkBuffer TextRenderer::textDrawParamsBuffer;
VkDeviceMemory TextRenderer::textDrawParamsBufferMemory;
uint32_t TextRenderer::textDrawParamsMemorySize;

VkImage TextRenderer::fontAtlasImage;
VkDeviceMemory TextRenderer::fontAtlasMemory;
VkImageView TextRenderer::fontAtlasImageView;
VkSampler TextRenderer::fontAtlasSampler;

VkDescriptorSetLayout TextRenderer::textDescriptorSetLayout;
VkDescriptorPool TextRenderer::textDescriptorPool;
std::vector<VkDescriptorSet> TextRenderer::textDescriptorSets;

VkPipelineLayout TextRenderer::pipelineLayout;
VkPipeline TextRenderer::textGraphicsPipeline;

uint32_t TextRenderer::longestTextSeen = 0;

void TextRenderer::init() {
    createFontAtlasVkImage();
    getFontAtlasGlyphs();

    GameRenderer::createUniformBuffers(textUniformBuffers, textUniformBuffersMemory, textUniformBuffersMapped);
    descriptorInit();

    const std::vector<char> vertShaderCode = GameRenderer::readFile("src/rendering/shaders/text_vert.spv");
    const std::vector<char> fragShaderCode = GameRenderer::readFile("src/rendering/shaders/text_frag.spv");
    GameRenderer::createGraphicsPipeline(
        pipelineLayout, textGraphicsPipeline,
        vertShaderCode, fragShaderCode,
        TexturedVertex::getBindingDescription(),
        TexturedVertex::getAttributeDescriptions(),
        textDescriptorSetLayout,
        false);
}

void TextRenderer::createQuadBuffers(uint32_t textSize) {
    if (textQuadVertices.empty()) {
        return;
    }

    if (textVertexBuffer != nullptr) {
        updateVertexBuffer();
    }

    if (textSize <= longestTextSeen) {
        return;
    }

    createVertexBuffer();
    createIndexBuffer(textSize);
    longestTextSeen = textSize;
}

void TextRenderer::updateVertexBuffer() {
    const uint32_t bufferSize = sizeof(TexturedVertex) * textQuadVertices.size();

    void* data;
    vkMapMemory(GameRenderer::device, textStagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, textQuadVertices.data(), bufferSize);
    vkUnmapMemory(GameRenderer::device, textStagingBufferMemory);

    GameRenderer::copyBuffer(textStagingBuffer, textVertexBuffer, bufferSize);
}

void TextRenderer::createVertexBuffer() {
    const uint32_t bufferSize = sizeof(TexturedVertex) * textQuadVertices.size();
    GameRenderer::destroyBuffer(textVertexBuffer, textVertexBufferMemory);
    GameRenderer::createVertexBuffer(textVertexBuffer, textVertexBufferMemory, bufferSize, textQuadVertices);

    GameRenderer::destroyBuffer(textStagingBuffer, textStagingBufferMemory);
    GameRenderer::createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        textStagingBuffer,
        textStagingBufferMemory);

    textVertexMemorySize = bufferSize;
}

void TextRenderer::createIndexBuffer(uint32_t textSize) {
    GameRenderer::destroyBuffer(textIndexBuffer, textIndexBufferMemory);

    for (uint32_t i = longestTextSeen; i < textSize; i++) {
        std::vector<uint32_t> newCharIndices = generateTexturedQuadIndices(i * 4);
        textQuadIndices.insert(textQuadIndices.end(), newCharIndices.begin(), newCharIndices.end());
    }

    const uint32_t indexBufferSize = sizeof(uint32_t) * textQuadIndices.size();
    GameRenderer::createIndexBuffer(textIndexBuffer, textIndexBufferMemory, indexBufferSize, textQuadIndices);
}

void TextRenderer::getFontAtlasGlyphs() {
    std::string pathToCSV = fontPath + "/" + "generated/" + fontToUse + ".csv";
    std::ifstream csvFile(pathToCSV);

    if (!csvFile.is_open()) {
        throw std::runtime_error("failed to open csv with font information!");
    }

    std::string line;

    while (std::getline(csvFile, line)) {
        std::ranges::replace(line, ',', ' ');
        std::stringstream ss(line);

        int charID;
        double advance, planeL, planeB, planeR, planeT, atlasL, atlasB, atlasR, atlasT;
        ss >> charID >> advance >> planeL >> planeB >> planeR >> planeT >> atlasL >> atlasB >> atlasR >> atlasT;
        char asciiChar = static_cast<char>(charID);

        Character characterInfo = {
            glm::vec4(planeL, planeB, planeR, planeT),
            glm::vec4(atlasL/atlasWidth, 1 - atlasB/atlasHeight, atlasR/atlasWidth, 1 - atlasT/atlasHeight),
            advance
        };

        characters.insert({asciiChar, characterInfo});
    }
}

void TextRenderer::createFontAtlasVkImage() {
    int channels;
    std::string pathToAtlas = fontPath + "/" + "generated/" + fontToUse + ".png";
    stbi_uc* pixels = stbi_load(pathToAtlas.c_str(), &atlasWidth, &atlasHeight, &channels, STBI_rgb_alpha);

    uint32_t imageSize = atlasWidth * atlasHeight * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    GameRenderer::createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(GameRenderer::device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(GameRenderer::device, stagingBufferMemory);

    GameRenderer::createImage(atlasWidth, atlasHeight,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        fontAtlasImage, fontAtlasMemory);

    GameRenderer::transitionImageLayout(fontAtlasImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    GameRenderer::copyBufferToImage(stagingBuffer, fontAtlasImage, atlasWidth, atlasHeight);

    GameRenderer::transitionImageLayout(fontAtlasImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    GameRenderer::destroyBuffer(stagingBuffer, stagingBufferMemory);

    fontAtlasImageView = GameRenderer::createTextureImageView(fontAtlasImage);
    GameRenderer::createTextureSampler(fontAtlasSampler);
}

void TextRenderer::descriptorInit() {
    GameRenderer::createDescriptorSetLayout(textDescriptorSetLayout, true);
    createTextDescriptorPool();
    createTextDescriptorSets();
}

void TextRenderer::createTextDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(GameRenderer::device, &poolInfo, nullptr, &textDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void TextRenderer::createTextDescriptorSets() {
        std::vector layouts(MAX_FRAMES_IN_FLIGHT, textDescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = textDescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        textDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(GameRenderer::device, &allocInfo, textDescriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = textUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = fontAtlasImageView;
            imageInfo.sampler = fontAtlasSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = textDescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = textDescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(GameRenderer::device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }
}

void TextRenderer::recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
    if (textQuadVertices.empty()) {
        return;
    }

    VkBuffer vertexBuffers[] = {textVertexBuffer};
    VkDeviceSize offsets[] = {0};

    UniformBufferObject ubo = Camera::ubo;
    memcpy(textUniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, textGraphicsPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, textIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
        0, 1, &textDescriptorSets[currentFrame], 0, nullptr);

    uint32_t drawCount = activeText.size();
    if (createDrawParamsBuffer(drawCount)) {
        vkCmdDrawIndexedIndirect(commandBuffer,
            textDrawParamsBuffer,
            0,
            drawCount,
            sizeof(VkDrawIndexedIndirectCommand));
    }
}

bool TextRenderer::createDrawParamsBuffer(uint32_t drawCount) {
    const VkDeviceSize bufferSize = sizeof(VkDrawIndexedIndirectCommand) * drawCount;

    if (bufferSize == 0) {
        return false;
    }

    if (bufferSize != textDrawParamsMemorySize) {
        GameRenderer::destroyBuffer(textDrawParamsBuffer, textDrawParamsBufferMemory);
        GameRenderer::createBuffer(bufferSize,
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            textDrawParamsBuffer,
            textDrawParamsBufferMemory);

        textDrawParamsMemorySize = bufferSize;
    }

    void* data;
    vkMapMemory(GameRenderer::device, textDrawParamsBufferMemory, 0, bufferSize, 0, &data);

    uint32_t commandIndex = 0;
    int vertexOffset = 0;
    for (auto& textToRender : activeText) {
        VkDrawIndexedIndirectCommand command;
        command.indexCount = static_cast<int>(textToRender.text.size()) * 6;
        command.instanceCount = 1;
        command.firstIndex = 0;
        command.vertexOffset = vertexOffset;
        command.firstInstance = 0;

        memcpy(static_cast<char*>(data) + commandIndex * sizeof(VkDrawIndexedIndirectCommand),
            &command,
            sizeof(VkDrawIndexedIndirectCommand));

        commandIndex++;
        vertexOffset += static_cast<int>(textToRender.text.size()) * 4;
    }

    vkUnmapMemory(GameRenderer::device, textDrawParamsBufferMemory);

    return true;
}

void TextRenderer::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        GameRenderer::destroyBuffer(textUniformBuffers[i], textUniformBuffersMemory[i]);
    }

    GameRenderer::destroyBuffer(textIndexBuffer, textIndexBufferMemory);
    GameRenderer::destroyBuffer(textVertexBuffer, textVertexBufferMemory);
    GameRenderer::destroyBuffer(textStagingBuffer, textStagingBufferMemory);
    GameRenderer::destroyBuffer(textDrawParamsBuffer, textDrawParamsBufferMemory);

    vkDestroyDescriptorPool(GameRenderer::device, textDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(GameRenderer::device, textDescriptorSetLayout, nullptr);
    vkDestroyPipeline(GameRenderer::device, textGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(GameRenderer::device, pipelineLayout, nullptr);
    vkDestroySampler(GameRenderer::device, fontAtlasSampler, nullptr);
    vkDestroyImageView(GameRenderer::device, fontAtlasImageView, nullptr);
    vkDestroyImage(GameRenderer::device, fontAtlasImage, nullptr);
    vkFreeMemory(GameRenderer::device, fontAtlasMemory, nullptr);
}

void TextRenderer::addText(const std::string& text, const glm::vec2& position, float scale, uint32_t id) {
    bool textFound = false;
    for (auto& textToRender : activeText) {
        if (textToRender.id == id) {
            textToRender = {text, position, scale, id};
            textFound = true;
        }
    }

    if (!textFound) {
        activeText.push_back({text, position, scale, id});
    }

    generateTextQuads();
    createQuadBuffers(text.size());
}

void TextRenderer::generateTextQuads() {
    textQuadVertices = {};
    textQuadIndices = {};

    for (auto& textToRender : activeText) {
        double currentAdvance = 0;
        for (auto& character : textToRender.text) {
            auto [planeQuad, atlasQuad, advance] = characters.at(character);

            glm::vec2 startPos = textToRender.position;

            float widthOffset = static_cast<float>(GameRenderer::getWidth())/2;
            float heightOffset = static_cast<float>(GameRenderer::getHeight())/2;

            startPos += glm::vec2(-widthOffset, -heightOffset);
            startPos[0] += static_cast<float>(currentAdvance);

            currentAdvance += advance * textToRender.scale;

            if (character == 32) {
                continue;
            }

            std::vector<TexturedVertex> charVertices = generateTexturedQuad(planeQuad, atlasQuad, startPos,
                textToRender.scale);

            //todo: cost is trivial but it could be more efficient to add a memory management system like my vertex pool
            textQuadVertices.insert(textQuadVertices.end(), charVertices.begin(), charVertices.end());
        }
    }
}