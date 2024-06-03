#include "TextRenderer.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "GameRenderer.h"

FT_Library TextRenderer::ft;
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
    std::vector<uint8_t> fontAtlas;
    uint32_t atlasWidth;
    uint32_t atlasHeight;

    if (!loadAtlasFromFile(fontAtlas, atlasWidth, atlasHeight)) {
        createFontAtlas(fontAtlas, atlasWidth, atlasHeight);
        saveAtlasToFile(fontAtlas, atlasWidth, atlasHeight);
    }

    createFontAtlasVkImage(fontAtlas, atlasWidth, atlasHeight);

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

void TextRenderer::createFontAtlas(std::vector<uint8_t>& fontAtlas, uint32_t& atlasWidth, uint32_t& atlasHeight) {
    if (FT_Init_FreeType(&ft)) {
        throw std::runtime_error("failed to initialize FreeType!");
    }

    std::string ttfFontPath = fontPath + "/" + fontToUse + ".ttf";

    FT_Face face;
    if (FT_New_Face(ft, ttfFontPath.c_str(), 0, &face)) {
        throw std::runtime_error("failed to load in the slkscr.ttf font!");
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);

    uint32_t totalWidth = 0;
    uint32_t maxWidth = 0;
    uint32_t maxHeight = 0;
    generateFontAtlasGlyphs(face, totalWidth, maxWidth, maxHeight);

    double requiredBins = static_cast<double>(totalWidth*maxHeight)/(maxWidth*maxHeight);
    auto binsPerSide = static_cast<uint32_t>(std::round(std::sqrt(requiredBins))) + 1; //todo verify and optimize this solution
    atlasWidth = binsPerSide * maxWidth;
    atlasHeight = binsPerSide * maxHeight;

    fontAtlas.resize(atlasWidth * atlasHeight * 4);
    createFontAtlasImage(face, fontAtlas, atlasWidth, maxHeight);

    if (FT_Load_Char(face, 43, FT_LOAD_RENDER))
    {
        throw std::runtime_error("failed to load in glyph!");
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::generateFontAtlasGlyphs(FT_Face face, uint32_t& totalWidth, uint32_t& maxWidth, uint32_t& maxHeight) {
    for (unsigned char c = 33; c < 127; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            throw std::runtime_error("failed to load in glyph!");
        }

        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);

        //save character information for use in drawing quads later
        Character character = {
            glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x,
            totalWidth
        };
        characters.insert(std::pair<char, Character>(c, character));

        totalWidth += face->glyph->bitmap.width;
        maxWidth = std::max(face->glyph->bitmap.width, maxHeight);
        maxHeight = std::max(face->glyph->bitmap.rows, maxHeight);
    }
}

void TextRenderer::createFontAtlasImage(FT_Face face, std::vector<uint8_t>& fontAtlas, uint32_t atlasWidth, uint32_t maxHeight) {
    uint32_t rowOffsetSize = atlasWidth * 4;
    uint32_t widthOffset = 0;
    uint32_t rowOffset = 0;

    for (unsigned char c = 33; c < 127; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            throw std::runtime_error("failed to load in glyph!");
        }

        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);

        uint32_t cols = face->glyph->bitmap.width;
        uint32_t rows = face->glyph->bitmap.rows;
        int pitch = abs(face->glyph->bitmap.pitch);

        if (widthOffset/4 + cols > atlasWidth) {
            widthOffset = 0;
            rowOffset += maxHeight * rowOffsetSize;
        }

        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                uint8_t pixel = face->glyph->bitmap.buffer[y * pitch + x];
                uint32_t atlasIndex = y * rowOffsetSize + x * 4 + widthOffset + rowOffset;

                //set four values for RGBA
                fontAtlas[atlasIndex] = pixel;
                fontAtlas[atlasIndex + 1] = pixel;
                fontAtlas[atlasIndex + 2] = pixel;
                fontAtlas[atlasIndex + 3] = 255;
            }
        }

        widthOffset += face->glyph->bitmap.width * 4;
    }
}

void TextRenderer::createFontAtlasVkImage(const std::vector<unsigned char>& fontAtlas,
    uint32_t atlasWidth, uint32_t atlasHeight) {
    uint32_t imageSize = atlasWidth * atlasHeight * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    GameRenderer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(GameRenderer::device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, fontAtlas.data(), imageSize);
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

void TextRenderer::saveAtlasToFile(const std::vector<uint8_t>& fontAtlas, uint32_t atlasWidth, uint32_t atlasHeight) {
    std::string fontAtlasPath = fontPath + "/generated/" + fontToUse + ".bin";

    std::filesystem::path directory = std::filesystem::path(fontAtlasPath).parent_path();
    if (!exists(directory)) {
        create_directories(directory);
    }

    std::ofstream outFile(fontAtlasPath, std::ios::binary);

    if (!outFile.is_open()) {
        throw std::runtime_error("failed to open file for saving font atlas! path: " + fontAtlasPath);
    }

    //store the width and height first in the file
    outFile.write(reinterpret_cast<const char*>(&atlasWidth), sizeof(uint32_t));
    outFile.write(reinterpret_cast<const char*>(&atlasHeight), sizeof(uint32_t));

    // store the raw atlas data
    outFile.write(reinterpret_cast<const char*>(fontAtlas.data()), static_cast<std::streamsize>(fontAtlas.size()));

    outFile.close();
}

bool TextRenderer::loadAtlasFromFile(std::vector<uint8_t>& fontAtlas, uint32_t& atlasWidth, uint32_t& atlasHeight) {
    std::string fontAtlasPath = fontPath + "/generated/" + fontToUse + ".bin";

    std::ifstream inFile(fontAtlasPath, std::ios::binary);

    if (!inFile.is_open()) {
        std::cerr << "existing font atlas not found, recreating...";
        return false;
    }

    // read in atlas dimensions
    inFile.read(reinterpret_cast<char*>(&atlasWidth), sizeof(uint32_t));
    inFile.read(reinterpret_cast<char*>(&atlasHeight), sizeof(uint32_t));
    size_t imageSize = atlasWidth * atlasHeight * 4;

    // read in the raw image data
    fontAtlas.resize(imageSize);
    inFile.read(reinterpret_cast<char*>(fontAtlas.data()), static_cast<std::streamsize>(imageSize));

    inFile.close();
    return true;
}