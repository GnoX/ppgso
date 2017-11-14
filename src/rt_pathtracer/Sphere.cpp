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
            glm::uvec2 uv = get_uv(pt);
            glm::vec3 n;
            if (material.normal_map.empty()) {
                n = glm::normalize(pt - center);
            } else {
                Spectrum s = material.normal_map.get_pixel(uv.x, uv.y);
                n = {s.r, s.g, s.b};
            }
            return {t, pt, n, &material, uv};
        }

        t = (-b + e) / a;

        if ( t > EPS ) {
            glm::vec3 pt = ray.point(t);
            glm::vec3 n;
            glm::uvec2 uv = get_uv(pt);
            if (material.normal_map.empty()) {
                n = glm::normalize(pt - center);
            } else {
                Spectrum s = material.normal_map.get_pixel(uv.x, uv.y);
                n = {s.r, s.g, s.b};
            }
            return {t, pt, n, &material, uv};
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
    return {center - glm::vec3(radius, radius, radius), center + glm::vec3(radius, radius, radius)};
}

glm::uvec2 Sphere::get_uv(glm::vec3 pt) const {
    unsigned w, h;
    if (!material.diffuse_map.empty()) {
        w = material.diffuse_map.w;
        h = material.diffuse_map.h;
    } else if (!material.normal_map.empty()) {
        w = material.normal_map.w;
        h = material.diffuse_map.h;
    } else return {0, 0};

    pt = glm::normalize(pt - center);
    auto u = static_cast<float>(.5 + atan2(pt.z, pt.x) / (2 * M_PI));
    auto v = static_cast<float>(.5 - asin(pt.y) / M_PI);
    return glm::uvec2((w - 1) * u, (h - 1) * v);
}
