#include "TextRenderer.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "GameRenderer.h"

std::map<char, Character> TextRenderer::characters;
const std::string TextRenderer::fontPath = "resources/fonts";
const std::string TextRenderer::fontToUse = "slkscr";
const uint32_t TextRenderer::fontSize = 512;

VkBuffer TextRenderer::textVertexBuffer;
VkDeviceMemory TextRenderer::textVertexBufferMemory;

VkBuffer TextRenderer::textIndexBuffer;
VkDeviceMemory TextRenderer::textIndexBufferMemory;

std::vector<VkBuffer> TextRenderer::textUniformBuffers;
std::vector<VkDeviceMemory> TextRenderer::textUniformBuffersMemory;
std::vector<void*> TextRenderer::textUniformBuffersMapped;

VkImage TextRenderer::fontAtlasImage;
VkDeviceMemory TextRenderer::fontAtlasMemory;
VkImageView TextRenderer::fontAtlasImageView;
VkSampler TextRenderer::fontAtlasSampler;

VkDescriptorSetLayout TextRenderer::textDescriptorSetLayout;
VkDescriptorPool TextRenderer::textDescriptorPool;
std::vector<VkDescriptorSet> TextRenderer::textDescriptorSets;

VkRenderPass TextRenderer::renderPass;
VkPipelineLayout TextRenderer::pipelineLayout;
VkPipeline TextRenderer::textGraphicsPipeline;

void TextRenderer::init() {
    getFontAtlasGlyphs();
    createFontAtlasVkImage();

    const uint32_t vertexBufferSize = sizeof(TexturedVertex) * vertices.size();
    const uint32_t indexBufferSize = sizeof(uint32_t) * indices.size();
    GameRenderer::createUniformBuffers(textUniformBuffers, textUniformBuffersMemory, textUniformBuffersMapped);
    descriptorInit();
    GameRenderer::createVertexBuffer(textVertexBuffer, textVertexBufferMemory, vertexBufferSize, vertices);
    GameRenderer::createIndexBuffer(textIndexBuffer, textIndexBufferMemory, indexBufferSize, indices);
    GameRenderer::createRenderPass(renderPass);

    const std::vector<char> vertShaderCode = GameRenderer::readFile("src/rendering/shaders/text_vert.spv");
    const std::vector<char> fragShaderCode = GameRenderer::readFile("src/rendering/shaders/text_frag.spv");
    GameRenderer::createGraphicsPipeline(
        pipelineLayout, textGraphicsPipeline,
        vertShaderCode, fragShaderCode,
        TexturedVertex::getBindingDescription(),
        TexturedVertex::getAttributeDescriptions(),
        textDescriptorSetLayout);
}

void TextRenderer::getFontAtlasGlyphs() {
    std::string pathToCSV = fontPath + "/" + "generated/" + fontToUse + ".csv";
    std::ifstream csvFile(pathToCSV);

    if (!csvFile.is_open()) {
        throw std::runtime_error("failed to open csv with font information!");
        return;
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
            glm::vec4(atlasL, atlasB, atlasR, atlasT),
            advance,
            asciiChar
        };

        characters.insert({asciiChar, characterInfo});
    }
}

void TextRenderer::createFontAtlasVkImage() {
    int atlasWidth;
    int atlasHeight;
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
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    GameRenderer::copyBufferToImage(stagingBuffer, fontAtlasImage, atlasWidth, atlasHeight);

    GameRenderer::transitionImageLayout(fontAtlasImage,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(GameRenderer::device, stagingBuffer, nullptr);
    vkFreeMemory(GameRenderer::device, stagingBufferMemory, nullptr);

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
    VkBuffer vertexBuffers[] = {textVertexBuffer};
    VkDeviceSize offsets[] = {0};

    UniformBufferObject ubo = Camera::ubo;
    memcpy(textUniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, textGraphicsPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, textIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
        0, 1, &textDescriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, indices.size(), 1, 0, 0, 0);
}

void TextRenderer::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(GameRenderer::device, textUniformBuffers[i], nullptr);
        vkFreeMemory(GameRenderer::device, textUniformBuffersMemory[i], nullptr);
    }

    vkDestroyBuffer(GameRenderer::device, textIndexBuffer, nullptr);
    vkFreeMemory(GameRenderer::device, textIndexBufferMemory, nullptr);

    vkDestroyBuffer(GameRenderer::device, textVertexBuffer, nullptr);
    vkFreeMemory(GameRenderer::device, textVertexBufferMemory, nullptr);

    vkDestroyDescriptorPool(GameRenderer::device, textDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(GameRenderer::device, textDescriptorSetLayout, nullptr);
    vkDestroySampler(GameRenderer::device, fontAtlasSampler, nullptr);
    vkDestroyImageView(GameRenderer::device, fontAtlasImageView, nullptr);
    vkDestroyImage(GameRenderer::device, fontAtlasImage, nullptr);
    vkFreeMemory(GameRenderer::device, fontAtlasMemory, nullptr);
}

