#ifndef VULKANUTIL_H
#define VULKANUTIL_H

#include <vector>
#include <vulkan/vulkan_core.h>

extern void createInstance(VkInstance& instance);

// SUPPORT/QUERY FUNCTIONS

static std::vector<const char*> getRequiredExtensions();

#endif //VULKANUTIL_H
