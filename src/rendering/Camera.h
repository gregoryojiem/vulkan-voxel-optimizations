#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

#include "TimeManager.h"
#include "../input/InputHandler.h"

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Camera {
public:
    void init(uint32_t width, uint32_t height);
    void update(std::vector<void*> uniformBuffersMapped, uint32_t currentImage, uint32_t width, uint32_t height, float deltaTime);

private:
    TimeManager timeManager;
    UniformBufferObject ubo;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    float yaw;
    float pitch;
    float fovy;
    float movementSpeed;
    float horzMouseSens;
    float vertMouseSens;
};

#endif //CAMERA_H
