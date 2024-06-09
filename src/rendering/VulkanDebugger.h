#ifndef VULKANDEBUGGER_H
#define VULKANDEBUGGER_H
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanDebugger {
public:
    static void init(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    static void cleanup(VkInstance& instance);

    static bool enabled();
    static void checkValidationLayerSupport();
    static void setupDebugMessenger(const VkInstance& instance);

    static const char* const* getValidationLayers();
    static uint32_t getValidationLayerSize();

private:
    static VkDebugUtilsMessengerEXT debugMessenger;
    static const std::vector<const char *> validationLayers;

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
                                             VkDebugUtilsMessengerEXT* pDebugMessenger);

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator);
};



#endif //VULKANDEBUGGER_H
