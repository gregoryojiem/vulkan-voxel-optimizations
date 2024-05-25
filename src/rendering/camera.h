#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

void updateUniformBuffer(std::vector<void*> uniformBuffersMapped, uint32_t currentImage, uint32_t width, uint32_t height);

#endif //CAMERA_H
