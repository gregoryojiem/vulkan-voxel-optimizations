#ifndef CAMERA_H
#define CAMERA_H

#include "VulkanStructs.h"
#include "GLFW/glfw3.h"

class Camera {
public:
    static UniformBufferObject ubo;

    void init(GLFWwindow *window, uint32_t width, uint32_t height);

    void update(float deltaTime);

    void updateProj(uint32_t width, uint32_t height) const;

private:
    glm::vec3 position{};
    glm::vec3 front{};
    glm::vec3 up{};
    glm::vec3 right{};
    float yaw{};
    float pitch{};
    float fovy{};
    float movementSpeed{};
    float horzMouseSens{};
    float vertMouseSens{};
};

#endif //CAMERA_H
