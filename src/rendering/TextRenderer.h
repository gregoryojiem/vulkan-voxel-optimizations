#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <map>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>

#include "Vertex.h"

struct Character {
    glm::vec4 planeQuad; //in the form of left, bottom, right top, with left bottom being on the baseline
    glm::vec4 atlasQuad;
    double advance;
};

struct ScreenText {
    std::string text;
    glm::vec2 position;
    float scale;
    uint32_t id;
};

class TextRenderer {
public:
    static void init();
    static void cleanup();
    static void recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);

    static void addText(const std::string& text, const glm::vec2& position, float scale, uint32_t id);
    static void generateTextQuads();

private:
    static int atlasWidth;
    static int atlasHeight;

    static std::vector<TexturedVertex> textQuadVertices;
    static std::vector<uint32_t> textQuadIndices;

    static std::vector<ScreenText> activeText;
    static std::map<char, Character> characters;
    const static std::string fontPath;
    const static std::string fontToUse;

    static VkBuffer textVertexBuffer;
    static VkDeviceMemory textVertexBufferMemory;
    static uint32_t textVertexMemorySize;
    static VkBuffer textStagingBuffer;
    static VkDeviceMemory textStagingBufferMemory;

    static VkBuffer textIndexBuffer;
    static VkDeviceMemory textIndexBufferMemory;

    static std::vector<VkBuffer> textUniformBuffers;
    static std::vector<VkDeviceMemory> textUniformBuffersMemory;
    static std::vector<void*> textUniformBuffersMapped;

    static VkBuffer textDrawParamsBuffer;
    static VkDeviceMemory textDrawParamsBufferMemory;
    static uint32_t textDrawParamsMemorySize;

    static VkImage fontAtlasImage;
    static VkDeviceMemory fontAtlasMemory;
    static VkImageView fontAtlasImageView;
    static VkSampler fontAtlasSampler;

    static VkDescriptorSetLayout textDescriptorSetLayout;
    static VkDescriptorPool textDescriptorPool;
    static std::vector<VkDescriptorSet> textDescriptorSets;

    static VkPipelineLayout pipelineLayout;
    static VkPipeline textGraphicsPipeline;

    static uint32_t longestTextSeen;

    static void getFontAtlasGlyphs();

    static void createQuadBuffers(uint32_t textSize);
    static void updateVertexBuffer();
    static void createVertexBuffer();
    static void createIndexBuffer(uint32_t textSize);
    static void descriptorInit();
    static void createTextDescriptorPool();
    static void createTextDescriptorSets();
    static void createFontAtlasVkImage();
    static bool createDrawParamsBuffer(uint32_t drawCount);
};

#endif //TEXTRENDERER_H
