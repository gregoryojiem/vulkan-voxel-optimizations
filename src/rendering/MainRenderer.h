#ifndef MAINRENDERER_H
#define MAINRENDERER_H
#include "CoreRenderer.h"
#include "ChunkRenderer.h"
#include "TextRenderer.h"
#include "Camera.h"
#include "../util/TimeManager.h"

class MainRenderer {
public:
    void init();

    void draw();

    void cleanup();

    GLFWwindow *getWindow();

private:
    Camera camera;
    TimeManager timeManager;
    CoreRenderer coreRenderer;
    ChunkRenderer chunkRenderer;
    TextRenderer textRenderer;
};

#endif //MAINRENDERER_H
