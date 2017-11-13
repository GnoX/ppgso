#ifndef PPGSO_GLRENDERER_H
#define PPGSO_GLRENDERER_H

#include <shader.h>
#include "src/rt_pathtracer/Renderer.h"
#include "pt_shaders/color_frag_glsl.h"
#include "pt_shaders/color_vert_glsl.h"

namespace pathtracer { namespace gl {

class GLRenderer : public Renderer {
public:
    ppgso::Shader color_shader = ppgso::Shader(color_vert_glsl, color_frag_glsl);

    void render(Scene &scene, bool updated) override;
    void stop() override;

};

}}


#endif //PPGSO_GLRENDERER_H
