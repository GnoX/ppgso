#include <mesh.h>
#include "GLRenderer.h"

namespace pathtracer {
    namespace gl {
        void GLRenderer::render(bool updated) {
            color_shader.use();
            color_shader.setUniform("ProjectionMatrix", scene->camera.projection);
            color_shader.setUniform("ModelMatrix", glm::mat4{});
            color_shader.setUniform("ViewMatrix", scene->camera.view);
            color_shader.setUniform("Color", glm::vec3{1, 0, 0});

            for (auto &object : scene->objects) {
                object->render();
            }
        }

        void GLRenderer::stop() {
            // TODO
        }

        void GLRenderer::set_scene(Scene *scene) {
            this->scene = scene;
        }
    }
}
