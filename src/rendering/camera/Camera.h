#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

#include "../VulkanUtil.h"
#include "../../utility/TimeManager.h"
#include "../../utility/InputHandler.h"

class Camera {
public:
    static UniformBufferObject ubo;

    void init(uint32_t width, uint32_t height);
    void update(float deltaTime);
    void updateProj(uint32_t width, uint32_t height);

private:
    TimeManager timeManager;
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
    float textScale;
};

#endif //CAMERA_H
