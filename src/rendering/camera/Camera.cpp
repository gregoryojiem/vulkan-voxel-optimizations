#include "Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cstring>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Camera::init(uint32_t width, uint32_t height) {
    position = glm::vec3(0.0f, 0.0f, 5.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = 0.0f;
    pitch = 0.0f;
    fovy = 45.0f;
    movementSpeed = 5.0f;
    horzMouseSens = 0.05f;
    vertMouseSens = 0.04f;

    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(position, glm::vec3(0.0f, 2.0f, 0.0f), up);
    ubo.proj = glm::perspective(glm::radians(fovy), width / (float) height, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
}


void Camera::update(std::vector<void*> uniformBuffersMapped, uint32_t currentImage, uint32_t width, uint32_t height, float deltaTime) {
    InputState state = InputHandler::getState();

    // handle input and calculate new yaw and pitch to get the new front vector
    yaw += -state.xDelta * horzMouseSens;
    pitch += state.yDelta * vertMouseSens;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    yaw = fmodf(yaw, 360);

    InputHandler::resetCursor();

    glm::vec3 front;
    front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
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

    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement);
    }

    position += movement * movementSpeed * deltaTime;
    ubo.view = glm::lookAt(position, position - front, up);

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
