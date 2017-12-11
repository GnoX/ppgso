#ifndef PPGSO_SPHERE_H
#define PPGSO_SPHERE_H


#include "TraceableObject.h"
#include "AABB.h"

class Sphere : public TraceableObject {
public:
    const Material material;

    double radius;
    glm::vec3 center;

    Sphere(double radius, glm::vec3 center, Material material);

    void render() const override;
    Intersection intersect(const Ray &ray) const override;

    AABB get_bbox() const override;
    glm::uvec2 get_uv(glm::vec3 point) const;

};


#endif //PPGSO_SPHERE_H
