#include "InputHandler.h"

InputState InputHandler::currentInputs;
bool InputHandler::mouseLocked = false;
double InputHandler::lastPosX;
double InputHandler::lastPosY;

int InputState::upState() const {
    return spaceState && !shiftState;
}

int InputState::downState() const {
    return spaceState && shiftState;
}

void InputHandler::init(GLFWwindow* window) {
    lockMouse(window);

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetKeyCallback(window, updateKeyboard);
    glfwSetMouseButtonCallback(window, updateMouse);
    glfwSetCursorPosCallback(window, updateCursor);
}

void InputHandler::updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!mouseLocked) { //todo maybe change this to while window visible
        return;
    }

    switch (key) {
        case GLFW_KEY_W:
            currentInputs.wState = action;
            break;
        case GLFW_KEY_A:
            currentInputs.aState = action;
            break;
        case GLFW_KEY_S:
            currentInputs.sState = action;
            break;
        case GLFW_KEY_D:
            currentInputs.dState = action;
            break;
        case GLFW_KEY_SPACE:
            currentInputs.spaceState = action;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            currentInputs.shiftState = action;
            break;
        default:
            break;
    }
}

void InputHandler::updateMouse(GLFWwindow* window, int button, int action, int mods) {
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            currentInputs.lmbState = action;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            currentInputs.rmbState = action;
            break;
        default:
            break;
    }
}

void InputHandler::updateCursor(GLFWwindow* window, double xpos, double ypos)
{
    currentInputs.xDelta = xpos - lastPosX;
    currentInputs.yDelta = ypos - lastPosY;
    lastPosX = xpos;
    lastPosY = ypos;
    if (currentInputs.xDelta > 100) { //todo remove once title is set-up
        currentInputs.xDelta = 0;
        currentInputs.yDelta = 0;
    }
}

void InputHandler::resetCursor() {
    currentInputs.xDelta = 0;
    currentInputs.yDelta = 0;
}

InputState InputHandler::getState() {
    return currentInputs;
}

void InputHandler::lockMouse(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    mouseLocked = true;
}

void InputHandler::unlockMouse(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    mouseLocked = false;
}
