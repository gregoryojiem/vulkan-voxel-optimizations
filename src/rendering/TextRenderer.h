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
    char id;
};

class TextRenderer {
public:
    static void init();
    static void cleanup();
    static void recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);

private:
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

    static void getFontAtlasGlyphs();
    static void createFontAtlasVkImage();

    static void descriptorInit();
    static void createTextDescriptorPool();
    static void createTextDescriptorSets();
};

const int atlasSize = 512/2;
const std::vector<TexturedVertex> vertices = {
    {{atlasSize * -1.0f, atlasSize * -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{atlasSize * 1.0f, atlasSize * -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{atlasSize * 1.0f, atlasSize * 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{atlasSize * -1.0f, atlasSize * 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
};

const std::vector<uint32_t> indices = {
    0, 1, 2,
    2, 3, 0
};


#endif //TEXTRENDERER_H
