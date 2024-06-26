#include "Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cstring>
#include <iostream>
#include <../../dependencies/glm-1.0.1/glm/glm.hpp>
#include <../../dependencies/glm-1.0.1/glm/gtc/matrix_transform.hpp>

#include "../../util/InputManager.h"
#include "../vulkan/VulkanStructs.h"

UniformBufferObject Camera::ubo;

void Camera::init(GLFWwindow *window, uint32_t width, uint32_t height) {
    position = glm::vec3(0.0f, 0.0f, 5.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = 0.0f;
    pitch = 0.0f;
    fovy = 45.0f;
    movementSpeed = 30.0f;
    horzMouseSens = 200.0f;
    vertMouseSens = 200.0f;

    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(position, glm::vec3(0.0f, 2.0f, 0.0f), up);
    ubo.proj = glm::perspective(glm::radians(fovy), aspectRatio, 0.1f, 10000.0f);
    ubo.proj[1][1] *= -1;
    InputHandler::init(window);
}


void Camera::update(float deltaTime) {
    InputState state = InputHandler::getState();

    // handle input and calculate new yaw and pitch to get the new front vector
    yaw += -static_cast<float>(state.xDelta) * horzMouseSens * deltaTime;
    pitch += static_cast<float>(state.yDelta) * vertMouseSens * deltaTime;
    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    yaw = fmodf(yaw, 360);

    InputHandler::resetCursor();

    glm::vec3 front;
    front.x = static_cast<float>(glm::sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
    front.y = glm::sin(glm::radians(pitch));
    front.z = static_cast<float>(glm::cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, up));

    // change position and update camera

    auto movement = glm::vec3(0.0f);

    if (state.wState) movement -= front;
    if (state.sState) movement += front;
    if (state.aState) movement += right;
    if (state.dState) movement -= right;
    if (state.spaceState) movement += up;
    if (state.shiftState) movement -= up;

    if (length(movement) > 0.0f) {
        movement = glm::normalize(movement);
    }

    position += movement * movementSpeed * deltaTime;
    ubo.view = glm::lookAt(position, position - front, up);
}

void Camera::updateProj(uint32_t width, uint32_t height) const {
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    ubo.proj = glm::perspective(glm::radians(fovy), aspectRatio, 0.1f, 300.0f);
}
