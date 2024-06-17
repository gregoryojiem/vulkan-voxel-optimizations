#ifndef MAINRENDERER_H
#define MAINRENDERER_H
#include "CoreRenderer.h"
#include "ChunkRenderer.h"
#include "TextRenderer.h"
#include "scene/Camera.h"

class MainRenderer {
public:
    void init();

    void draw();

    void cleanup() const;

    static GLFWwindow *getWindow();

private:
    Camera camera;
    CoreRenderer coreRenderer;
    ChunkRenderer chunkRenderer;
    TextRenderer textRenderer;
};

#endif //MAINRENDERER_H
