
#include "Triangle.h"

namespace pathtracer { namespace pt {
Triangle::Triangle(pathtracer::gl::Mesh *mesh, uint32_t v0, uint32_t v1, uint32_t v2)
        // : mesh(mesh), v0(v0), v1(v1), v2(v2) {}
{}

void Triangle::render() const {

}

Intersection Triangle::intersect(const Ray &ray) const {

    glm::vec3 v01 = v1 - v0;
    glm::vec3 v02 = v2 - v0;
    glm::vec3 pvec = glm::cross(ray.direction, v02);
    double det = glm::dot(v01, pvec);
    if (fabs(det) < EPS) return noHit;
    double invDet = 1 / det;
    glm::vec3 tvec = ray.origin - v0;
    double u = glm::dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1) return noHit;

    glm::vec3 qvec = glm::cross(tvec, v01);
    double v = glm::dot(ray.direction, qvec) * invDet;
    if (v < 0 || u + v > 1) return noHit;

    double t = glm::dot(v02, qvec) * invDet;
    if (t > EPS) {
        glm::vec3 pt = ray.point(t);
        glm::vec3 n = glm::normalize(glm::cross(v01, v02));
        return {t, pt, n, &material, get_uv(pt)};
    } else {
        return noHit;
    }
}

AABB Triangle::get_bbox() const {
    return {glm::min(glm::min(v0, v1), v2), glm::max(glm::max(v0, v1), v2)};
}

// TODO
glm::uvec2 Triangle::get_uv(glm::vec3 point) const {
    return glm::uvec2(0.0f, 0.0f);
}
}}
