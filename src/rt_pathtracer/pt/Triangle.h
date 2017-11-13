#ifndef PPGSO_TRIANGLE_H
#define PPGSO_TRIANGLE_H


#include <glm/vec3.hpp>
#include "src/rt_pathtracer/gl/Mesh.h"

namespace pathtracer { namespace pt {
    class Triangle {
        Triangle(pathtracer::gl::Mesh *mesh, uint32_t v0, uint32_t v1, uint32_t v2);

        void render();

        pathtracer::gl::Mesh *mesh;
        uint32_t v0, v1, v2;

    };
}}


#endif //PPGSO_TRIANGLE_H
