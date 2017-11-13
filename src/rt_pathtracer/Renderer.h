#ifndef PPGSO_RENDERER_H
#define PPGSO_RENDERER_H


#include "Scene.h"

class Renderer {

public:
    virtual void render(Scene &scene, bool updated) = 0;
    virtual void stop() = 0;
};


#endif //PPGSO_RENDERER_H
