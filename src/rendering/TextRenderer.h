#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <map>
#include <../../../dependencies/glm-1.0.1/glm/vec2.hpp>
#include <vulkan/vulkan.hpp>

#include "scene/Vertex.h"

struct Character {
    glm::vec4 planeQuad; //in the form of left, bottom, right top, with left bottom being on the baseline
    glm::vec4 atlasQuad;
    float advance;
};

struct ScreenText {
    std::string text;
    glm::vec2 position;
    float scale;
    uint32_t id;
};

class TextRenderer {
public:
    void init(VkDescriptorPool& descriptorPool, VkRenderPass renderPass);

    void addText(const std::string &text, const glm::vec2 &position, float scale, uint32_t id);

    void draw(const VkDevice &device, const VkCommandBuffer &commandBuffer, uint32_t currentFrame, float fps);

    void cleanup(const VkDevice &device) const;

private:
    const static std::string fontPath;
    const static std::string fontToUse;
    int atlasWidth{};
    int atlasHeight{};

    VkBuffer textVertexBuffer{};
    VkDeviceMemory textVertexBufferMemory{};
    uint32_t textVertexMemorySize{};
    VkBuffer textStagingBuffer{};
    VkDeviceMemory textStagingBufferMemory{};

    VkBuffer textIndexBuffer{};
    VkDeviceMemory textIndexBufferMemory{};

    VkBuffer textDrawParamsBuffer{};
    VkDeviceMemory textDrawParamsBufferMemory{};
    uint32_t textDrawParamsMemorySize{};

    VkImage fontAtlasImage{};
    VkDeviceMemory fontAtlasMemory{};
    VkImageView atlasImageView{};
    VkSampler atlasSampler{};

    VkDescriptorSetLayout descriptorSetLayout{};
    std::vector<VkDescriptorSet> descriptorSets;

    VkPipelineLayout pipelineLayout{};
    VkPipeline textGraphicsPipeline{};

    std::vector<TexturedVertex> textQuadVertices;
    std::vector<uint32_t> textQuadIndices;

    uint32_t longestTextSeen{};
    std::vector<ScreenText> activeText;
    std::map<char, Character> characters;

    void createFontAtlasGlyphs();

    void createTextQuads();

    void createQuadBuffers(uint32_t textSize);

    void createTextVertexBuffer();

    void createTextIndexBuffer(uint32_t textSize);

    bool createDrawParamsBuffer(const VkDevice &device, uint32_t drawCount);

    void printFPS(float fps);
};

#endif //TEXTRENDERER_H
