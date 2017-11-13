#ifndef PPGSO_TRACABLEOBJECT_H
#define PPGSO_TRACABLEOBJECT_H


#include <src/rt_pathtracer/pt/Ray.h>
#include <src/rt_pathtracer/pt/Intersection.h>
#include "Object.h"
#include "AABB.h"

struct TracableObject {
    virtual ~TracableObject() = default;

    virtual Intersection intersect(const Ray &ray) const = 0;
    virtual void render() const = 0;
    virtual AABB get_bbox() const = 0;
};


#endif //PPGSO_TRACABLEOBJECT_H
