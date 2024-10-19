#ifndef VULKANSTRUCTS_H
#define VULKANSTRUCTS_H
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Plane {
    glm::vec3 normal;
    glm::vec3 distFromOrigin;
};

struct Frustrum {
    Plane bottom;
    Plane top;
    Plane left;
    Plane right;
    Plane near;
    Plane far;
};


#endif //VULKANSTRUCTS_H
