#include "TextRenderer.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "vulkan/VulkanBufferUtil.h"
#include "vulkan/VulkanUtil.h"
#include "../util/GraphicsUtil.h"

const std::string TextRenderer::fontPath = "../resources/fonts";
const std::string TextRenderer::fontToUse = "abel";

void TextRenderer::init() {
    createShaderImageFromFile(fontAtlasImage, fontAtlasMemory, atlasWidth, atlasHeight,
                              fontPath + "/" + "generated/" + fontToUse + ".png");
    atlasImageView = createTextureImageView(fontAtlasImage);
    createTextureSampler(atlasSampler);
    createFontAtlasGlyphs();
    createDescriptorSetLayout(descriptorSetLayout, true);
    createDescriptorPool(descriptorPool, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    createDescriptorSetsSampler(descriptorSets, descriptorSetLayout, descriptorPool, atlasImageView, atlasSampler);
    createGraphicsPipeline(
        pipelineLayout, textGraphicsPipeline,
        "../src/rendering/shaders/text_vert.spv", "../src/rendering/shaders/text_frag.spv",
        TexturedVertex::getBindingDescription(),
        TexturedVertex::getAttributeDescriptions(), descriptorSetLayout,
        false);
}

void TextRenderer::createFontAtlasGlyphs() {
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
            static_cast<float>(advance)
        };
        characters.insert({asciiChar, characterInfo});
    }
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
    createTextQuads();
    createQuadBuffers(text.size());
}

void TextRenderer::createTextQuads() {
    textQuadVertices = {};
    textQuadIndices = {};
    for (auto &[text, position, scale, id]: activeText) {
        float currentAdvance = 0;
        const float xScale = scale / static_cast<float>(getWindowWidth());
        const float yScale = scale / static_cast<float>(getWindowHeight());

        for (auto &character: text) {
            auto [planeQuad, atlasQuad, advance] = characters.at(character);
            glm::vec2 startPos = position;
            startPos[0] += currentAdvance;
            currentAdvance += advance * xScale;
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

void TextRenderer::createQuadBuffers(uint32_t textSize) {
    if (textQuadVertices.empty()) {
        return;
    }
    if (textVertexBuffer != nullptr) {
        updateBuffer(textVertexBuffer, textStagingBuffer, textStagingBufferMemory,
                     textQuadVertices.data(), sizeof(TexturedVertex) * textQuadVertices.size());
    }
    if (textSize <= longestTextSeen) {
        return;
    }
    createTextVertexBuffer();
    createTextIndexBuffer(textSize);
    longestTextSeen = textSize;
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

void TextRenderer::draw(const VkDevice &device, const VkCommandBuffer &commandBuffer, uint32_t currentFrame,
                        float fps) {
    printFPS(fps);
    if (textQuadVertices.empty()) {
        return;
    }

    VkBuffer vertexBuffers[] = {textVertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, textGraphicsPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, textIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &descriptorSets[currentFrame], 0, nullptr);
    uint32_t drawCount = activeText.size();
    if (createDrawParamsBuffer(device, drawCount)) {
        vkCmdDrawIndexedIndirect(commandBuffer,
                                 textDrawParamsBuffer,
                                 0,
                                 drawCount,
                                 sizeof(VkDrawIndexedIndirectCommand));
    }
}

bool TextRenderer::createDrawParamsBuffer(const VkDevice &device, uint32_t drawCount) {
    const VkDeviceSize bufferSize = sizeof(VkDrawIndexedIndirectCommand) * drawCount;
    if (bufferSize == 0) {
        return false;
    }
    if (bufferSize != textDrawParamsMemorySize) {
        createIndirectBuffer(textDrawParamsBuffer, textDrawParamsBufferMemory, bufferSize);
        textDrawParamsMemorySize = bufferSize;
    }

    void *data;
    vkMapMemory(device, textDrawParamsBufferMemory, 0, bufferSize, 0, &data);

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
               &command, sizeof(VkDrawIndexedIndirectCommand));
        commandIndex++;
        vertexOffset += static_cast<int>(textToRender.text.size()) * 4;
    }

    vkUnmapMemory(device, textDrawParamsBufferMemory);
    return true;
}

void TextRenderer::printFPS(float fps) {
    if (fps < 0) {
        return;
    }
    std::stringstream ss;
    ss << "fps: " << fps;
    const std::string fpsString = ss.str();
    addText(fpsString, glm::vec2(-0.99f, -0.93f), 64.0f, 1);
}

void TextRenderer::cleanup(const VkDevice &device) const {
    destroyBuffer(textIndexBuffer, textIndexBufferMemory);
    destroyBuffer(textVertexBuffer, textVertexBufferMemory);
    destroyBuffer(textStagingBuffer, textStagingBufferMemory);
    destroyBuffer(textDrawParamsBuffer, textDrawParamsBufferMemory);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, textGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroySampler(device, atlasSampler, nullptr);
    vkDestroyImageView(device, atlasImageView, nullptr);
    vkDestroyImage(device, fontAtlasImage, nullptr);
    vkFreeMemory(device, fontAtlasMemory, nullptr);
}
