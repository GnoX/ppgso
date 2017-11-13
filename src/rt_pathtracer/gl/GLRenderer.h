#ifndef PPGSO_GLRENDERER_H
#define PPGSO_GLRENDERER_H

#include <shader.h>
#include "src/rt_pathtracer/Renderer.h"
#include "pt_shaders/simple_color_frag_glsl.h"
#include "pt_shaders/simple_color_vert_glsl.h"

namespace pathtracer { namespace gl {

class GLRenderer : public Renderer {
public:
    void render(Scene &scene) override;

    ppgso::Shader color_shader = ppgso::Shader(simple_color_vert_glsl, simple_color_frag_glsl);
};

}}


#endif //PPGSO_GLRENDERER_H
