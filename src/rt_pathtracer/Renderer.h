#ifndef PPGSO_RENDERER_H
#define PPGSO_RENDERER_H


#include "Scene.h"

class Renderer {

public:
    virtual void render(bool updated) = 0;
    virtual void stop() = 0;
    virtual void set_scene(Scene *scene) = 0;
};


#endif //PPGSO_RENDERER_H
