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
    Scene *scene;

    void render(bool updated) override;
    void stop() override;
    void set_scene(Scene *scene) override;

};

}}


#endif //PPGSO_GLRENDERER_H
