#ifndef CAMERA_H
#define CAMERA_H

#include "GLFW/glfw3.h"
#include "../vulkan/VulkanStructs.h"
#include "../../util/InputManager.h"

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

class Camera {
public:
    UniformBufferObject ubo;

    void init(GLFWwindow *window, uint32_t width, uint32_t height);

    void update(float deltaTime);

    void handleMouseInput(const InputState &state, float deltaTime);

    [[nodiscard]] glm::vec3 handleKeyboardInput(const InputState &state, const glm::vec3 &front, float deltaTime) const;

    void updateAspect(uint32_t width, uint32_t height);

private:
    glm::vec3 position{};
    glm::vec3 up{};
    float yaw{};
    float pitch{};
    float fovY{};
    float aspect{};
    float zNear{};
    float zFar{};
    float movementSpeed{};
    float horzMouseSens{};
    float vertMouseSens{};

    [[nodiscard]] glm::vec3 getFrontVec() const;
};

#endif //CAMERA_H
