#include <mesh.h>
#include "GLRenderer.h"

namespace pathtracer {
    namespace gl {
        void GLRenderer::render(Scene &scene) {
            color_shader.use();
            color_shader.setUniform("ProjectionMatrix", scene.camera.projection);
            color_shader.setUniform("ModelMatrix", glm::mat4{});
            color_shader.setUniform("ViewMatrix", scene.camera.view);
            color_shader.setUniform("Color", glm::vec3{1, 0, 0});

            for (auto &object : scene.objects) {
                object->render();
            }
        }
    }
}
