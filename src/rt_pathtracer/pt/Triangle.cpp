
#include "Triangle.h"

namespace pathtracer { namespace pt {
    Triangle::Triangle(pathtracer::gl::Mesh *mesh, uint32_t v0, uint32_t v1, uint32_t v2)
            : mesh(mesh), v0(v0), v1(v1), v2(v2) {}

    void Triangle::render() {

    }
}}
