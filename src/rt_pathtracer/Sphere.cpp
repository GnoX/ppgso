#include <cmath>
#include <glm/geometric.hpp>
#include <src/rt_pathtracer/pt/Intersection.h>
#include "Sphere.h"

void Sphere::render() const {

}

Intersection Sphere::intersect(const Ray &ray) const {
    glm::vec3 oc = ray.origin - center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;
    float dis = b * b - a * c;

    if (dis > 0) {
        float e = std::sqrt(dis);
        float t = (-b - e) / a;

        if ( t > EPS ) {
            glm::vec3 pt = ray.point(t);
            glm::vec3 n = glm::normalize(pt - center);
            return {t, pt, n, &material};
        }

        t = (-b + e) / a;

        if ( t > EPS ) {
            glm::vec3 pt = ray.point(t);
            glm::vec3 n = glm::normalize(pt - center);
            return {t, pt, n, &material};
        }
    }
    return noHit;
}

Sphere::Sphere(double radius, glm::vec3 center, const Material material)
        : material(material)
        , radius(radius)
        , center(center){

}

AABB Sphere::get_bbox() const {
    return AABB(center - glm::vec3(radius, radius, radius), center + glm::vec3(radius, radius, radius));
}
