#ifndef PPGSO_SPHERE_H
#define PPGSO_SPHERE_H


#include "TracableObject.h"

class Sphere : public TracableObject {
public:
    const Material material;

    double radius;
    glm::vec3 center;

    Sphere(double radius, glm::vec3 center, Material material);

    void render() const override;
    Intersection intersect(const Ray &ray) const override;


};


#endif //PPGSO_SPHERE_H
