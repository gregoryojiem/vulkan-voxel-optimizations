#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <windows.h>
#include <GLFW/glfw3.h>

struct InputState {
    int wState = 0;
    int aState = 0;
    int sState = 0;
    int dState = 0;
    int spaceState = 0;
    int shiftState = 0;
    int lmbState = 0;
    int rmbState = 0;
    double xDelta = 0;
    double yDelta = 0;

    [[nodiscard]] int upState() const;
    [[nodiscard]] int downState() const;
};

class InputHandler {
public:
    static InputState currentInputs;
    static bool mouseLocked;

    static void init(GLFWwindow* window);
    static void updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void updateMouse(GLFWwindow* window, int button, int action, int mods);
    static void updateCursor(GLFWwindow* window, double xpos, double ypos);
    static void resetCursor();

    static InputState getState();

    static void lockMouse(GLFWwindow* window);
    static void unlockMouse(GLFWwindow* window);

private:
    static double lastPosX;
    static double lastPosY;
};

#endif //INPUTHANDLER_H
