#include "TextRenderer.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanBufferUtil.h"
#include "VulkanUtil.h"
#include "CoreRenderer.h"
#include "../util/GraphicsUtil.h"
#include "../util/TimeManager.h"

int TextRenderer::atlasWidth;
int TextRenderer::atlasHeight;

std::vector<TexturedVertex> TextRenderer::textQuadVertices;
std::vector<uint32_t> TextRenderer::textQuadIndices;

std::vector<ScreenText> TextRenderer::activeText;
std::map<char, Character> TextRenderer::characters;
const std::string TextRenderer::fontPath = "../resources/fonts";
const std::string TextRenderer::fontToUse = "abel";

VkBuffer TextRenderer::textVertexBuffer;
VkDeviceMemory TextRenderer::textVertexBufferMemory;
uint32_t TextRenderer::textVertexMemorySize;
VkBuffer TextRenderer::textStagingBuffer;
VkDeviceMemory TextRenderer::textStagingBufferMemory;

VkBuffer TextRenderer::textIndexBuffer;
VkDeviceMemory TextRenderer::textIndexBufferMemory;

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

    descriptorInit();

    createGraphicsPipeline(
        pipelineLayout, textGraphicsPipeline,
        "../src/rendering/shaders/text_vert.spv", "../src/rendering/shaders/text_frag.spv",
        TexturedVertex::getBindingDescription(),
        TexturedVertex::getAttributeDescriptions(), textDescriptorSetLayout,
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

    createTextVertexBuffer();
    createTextIndexBuffer(textSize);
    longestTextSeen = textSize;
}

void TextRenderer::updateVertexBuffer() {
    const uint32_t bufferSize = sizeof(TexturedVertex) * textQuadVertices.size();

    void *data;
    vkMapMemory(CoreRenderer::device, textStagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, textQuadVertices.data(), bufferSize);
    vkUnmapMemory(CoreRenderer::device, textStagingBufferMemory);

    copyBuffer(textStagingBuffer, textVertexBuffer, bufferSize);
}

void TextRenderer::createTextVertexBuffer() {
    const uint32_t bufferSize = sizeof(TexturedVertex) * textQuadVertices.size();
    createVertexBuffer(textVertexBuffer, textVertexBufferMemory, bufferSize, textQuadVertices);
    createStagingBuffer(textStagingBuffer, textStagingBufferMemory, bufferSize);
    textVertexMemorySize = bufferSize;
}

void TextRenderer::createTextIndexBuffer(uint32_t textSize) {
    for (uint32_t i = longestTextSeen; i < textSize; i++) {
        std::vector<uint32_t> newCharIndices = generateTexturedQuadIndices(i * 4);
        textQuadIndices.insert(textQuadIndices.end(), newCharIndices.begin(), newCharIndices.end());
    }

    const uint32_t indexBufferSize = sizeof(uint32_t) * textQuadIndices.size();
    createIndexBuffer(textIndexBuffer, textIndexBufferMemory, indexBufferSize, textQuadIndices);
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
            glm::vec4(atlasL / atlasWidth, 1 - atlasB / atlasHeight, atlasR / atlasWidth, 1 - atlasT / atlasHeight),
            advance
        };

        characters.insert({asciiChar, characterInfo});
    }
}

void TextRenderer::createFontAtlasVkImage() {
    int channels;
    std::string pathToAtlas = fontPath + "/" + "generated/" + fontToUse + ".png";
    stbi_uc *pixels = stbi_load(pathToAtlas.c_str(), &atlasWidth, &atlasHeight, &channels, STBI_rgb_alpha);

    uint32_t imageSize = atlasWidth * atlasHeight * 4;

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    createStagingBuffer(stagingBuffer, stagingBufferMemory, imageSize);

    void *data;
    vkMapMemory(CoreRenderer::device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(CoreRenderer::device, stagingBufferMemory);

    createImage(fontAtlasImage, fontAtlasMemory, CoreRenderer::device, CoreRenderer::physicalDevice,
                atlasWidth,
                atlasHeight,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    transitionImageLayout(fontAtlasImage,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          CoreRenderer::device,
                          CoreRenderer::commandPool,
                          CoreRenderer::graphicsQueue);

    copyBufferToImage(stagingBuffer, fontAtlasImage, atlasWidth, atlasHeight);

    transitionImageLayout(fontAtlasImage,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          CoreRenderer::device,
                          CoreRenderer::commandPool,
                          CoreRenderer::graphicsQueue);

    destroyBuffer(stagingBuffer, stagingBufferMemory);

    fontAtlasImageView = createTextureImageView(fontAtlasImage, CoreRenderer::device);
    createTextureSampler(fontAtlasSampler, CoreRenderer::device, CoreRenderer::physicalDevice);
}

void TextRenderer::descriptorInit() {
    createDescriptorSetLayout(textDescriptorSetLayout, true);
    createTextDescriptorPool();
    createTextDescriptorSets();
}

void TextRenderer::createTextDescriptorPool() {
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(CoreRenderer::device, &poolInfo, nullptr, &textDescriptorPool) != VK_SUCCESS) {
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
    if (vkAllocateDescriptorSets(CoreRenderer::device, &allocInfo, textDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = fontAtlasImageView;
        imageInfo.sampler = fontAtlasSampler;

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = textDescriptorSets[i];
        descriptorWrites[0].dstBinding = 1;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(CoreRenderer::device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void TextRenderer::recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
    printFPS();

    if (textQuadVertices.empty()) {
        return;
    }

    VkBuffer vertexBuffers[] = {textVertexBuffer};
    VkDeviceSize offsets[] = {0};

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
        createIndirectBuffer(textDrawParamsBuffer, textDrawParamsBufferMemory, bufferSize);
        textDrawParamsMemorySize = bufferSize;
    }

    void *data;
    vkMapMemory(CoreRenderer::device, textDrawParamsBufferMemory, 0, bufferSize, 0, &data);

    uint32_t commandIndex = 0;
    int vertexOffset = 0;
    for (auto &textToRender: activeText) {
        VkDrawIndexedIndirectCommand command;
        command.indexCount = static_cast<int>(textToRender.text.size()) * 6;
        command.instanceCount = 1;
        command.firstIndex = 0;
        command.vertexOffset = vertexOffset;
        command.firstInstance = 0;

        memcpy(static_cast<char *>(data) + commandIndex * sizeof(VkDrawIndexedIndirectCommand),
               &command,
               sizeof(VkDrawIndexedIndirectCommand));

        commandIndex++;
        vertexOffset += static_cast<int>(textToRender.text.size()) * 4;
    }

    vkUnmapMemory(CoreRenderer::device, textDrawParamsBufferMemory);

    return true;
}

void TextRenderer::cleanup() {
    destroyBuffer(textIndexBuffer, textIndexBufferMemory);
    destroyBuffer(textVertexBuffer, textVertexBufferMemory);
    destroyBuffer(textStagingBuffer, textStagingBufferMemory);
    destroyBuffer(textDrawParamsBuffer, textDrawParamsBufferMemory);
    vkDestroyDescriptorPool(CoreRenderer::device, textDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(CoreRenderer::device, textDescriptorSetLayout, nullptr);
    vkDestroyPipeline(CoreRenderer::device, textGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(CoreRenderer::device, pipelineLayout, nullptr);
    vkDestroySampler(CoreRenderer::device, fontAtlasSampler, nullptr);
    vkDestroyImageView(CoreRenderer::device, fontAtlasImageView, nullptr);
    vkDestroyImage(CoreRenderer::device, fontAtlasImage, nullptr);
    vkFreeMemory(CoreRenderer::device, fontAtlasMemory, nullptr);
}

void TextRenderer::addText(const std::string &text, const glm::vec2 &position, float scale, uint32_t id) {
    bool textFound = false;
    for (auto &textToRender: activeText) {
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

    for (auto &textToRender: activeText) {
        float currentAdvance = 0;
        const float xScale = textToRender.scale/static_cast<float>(CoreRenderer::getWindowWidth());
        const float yScale = textToRender.scale/static_cast<float>(CoreRenderer::getWindowHeight());
        for (auto &character: textToRender.text) {
            auto [planeQuad, atlasQuad, advance] = characters.at(character);
            glm::vec2 startPos = textToRender.position;
            startPos[0] += currentAdvance;
            currentAdvance += static_cast<float>(advance) * xScale;
            if (character == 32) {
                continue;
            }
            planeQuad *= glm::vec4(xScale, yScale, xScale, yScale);
            std::vector<TexturedVertex> charVertices = generateTexturedQuad(planeQuad, atlasQuad, startPos);
            //todo: cost is trivial but it could be more efficient to add a memory management system like my vertex pool
            textQuadVertices.insert(textQuadVertices.end(), charVertices.begin(), charVertices.end());
        }
    }
}

void TextRenderer::printFPS() {
    const float fps = TimeManager::queryFPS();
    if (fps < 0) {
        return;
    }

    std::stringstream ss;
    ss << "fps: " << fps;
    const std::string fpsString = ss.str();
    addText(fpsString, glm::vec2(-0.99f, -0.93f), 64.0f, 1);
}
