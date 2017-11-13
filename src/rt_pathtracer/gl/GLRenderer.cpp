#include <mesh.h>
#include "GLRenderer.h"

namespace pathtracer {
    namespace gl {
        void GLRenderer::render(Scene &scene) {
            shader.use();
            shader.setUniform("ProjectionMatrix", glm::mat4{});
            shader.setUniform("ModelMatrix", glm::mat4{});
            shader.setUniform("ViewMatrix", glm::mat4{});
            for (auto &object : scene.objects) {
                object->render();
            }
        }
    }
}
