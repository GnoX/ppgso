#ifndef PPGSO_GLRENDERER_H
#define PPGSO_GLRENDERER_H

#include <shader.h>
#include "src/rt_pathtracer/Renderer.h"
#include "shaders/texture_frag_glsl.h"
#include "shaders/texture_vert_glsl.h"

namespace pathtracer { namespace gl {
    class GLRenderer : public Renderer {
    public:
        void render(Scene &scene) override;

        ppgso::Shader shader = ppgso::Shader(texture_vert_glsl, texture_frag_glsl);
    };
}}


#endif //PPGSO_GLRENDERER_H
