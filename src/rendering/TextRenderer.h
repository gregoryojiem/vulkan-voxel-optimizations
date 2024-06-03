#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>

#include "Vertex.h"

struct Character {
    glm::vec2   size;       // Size of glyph
    glm::vec2   bearing;    // Offset from baseline to left/top of glyph
    signed long advance;    // Offset to advance to next glyph
    uint32_t atlasOffset;
};

class TextRenderer {
public:
    static void init();
    static void cleanup();
    static void recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);

private:
    static FT_Library ft;
    static std::map<char, Character> characters;
    const static std::string fontPath;
    const static std::string fontToUse;
    const static uint32_t fontSize;

    static VkBuffer textVertexBuffer;
    static VkDeviceMemory textVertexBufferMemory;

    static VkBuffer textIndexBuffer;
    static VkDeviceMemory textIndexBufferMemory;

    static std::vector<VkBuffer> textUniformBuffers;
    static std::vector<VkDeviceMemory> textUniformBuffersMemory;
    static std::vector<void*> textUniformBuffersMapped;

    static VkImage fontAtlasImage;
    static VkDeviceMemory fontAtlasMemory;
    static VkImageView fontAtlasImageView;
    static VkSampler fontAtlasSampler;

    static VkDescriptorSetLayout textDescriptorSetLayout;
    static VkDescriptorPool textDescriptorPool;
    static std::vector<VkDescriptorSet> textDescriptorSets;

    static VkRenderPass renderPass;
    static VkPipelineLayout pipelineLayout;
    static VkPipeline textGraphicsPipeline;

    static void createFontAtlas(std::vector<uint8_t>& fontAtlas, uint32_t& atlasWidth, uint32_t& atlasHeight);
    static void generateFontAtlasGlyphs(FT_Face face, uint32_t& totalWidth, uint32_t& maxWidth, uint32_t& maxHeight);
    static void createFontAtlasImage(FT_Face face, std::vector<uint8_t>& fontAtlas, uint32_t atlasWidth, uint32_t maxHeight);
    static void createFontAtlasVkImage(const std::vector<uint8_t>& fontAtlas,
        uint32_t atlasWidth, uint32_t atlasHeight);

    static void descriptorInit();
    static void createTextDescriptorPool();
    static void createTextDescriptorSets();

    static void saveAtlasToFile(const std::vector<uint8_t>& fontAtlas, uint32_t atlasWidth, uint32_t atlasHeight);
    static bool loadAtlasFromFile(std::vector<uint8_t>& fontAtlas, uint32_t& atlasWidth, uint32_t& atlasHeight);
};

const std::vector<TexturedVertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
};

const std::vector<uint32_t> indices = {
    0, 1, 2,
    2, 3, 0
};


#endif //TEXTRENDERER_H
