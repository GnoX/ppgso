#ifndef PPGSO_AABB_H
#define PPGSO_AABB_H

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <src/rt_pathtracer/pt/Ray.h>
#include "defs.h"

class AABB {
public:
    glm::vec3 extent;
    glm::vec3 bounds[2];

    AABB() {
        bounds[0] = glm::vec3(INF);
        bounds[1] = glm::vec3(-INF);
        extent = bounds[0] - bounds[1];
    }
    AABB(glm::vec3 min, glm::vec3 max)  {
        bounds[0] = min;
        bounds[1] = max;
        extent = max - min;
    }

    float surface_area() const {
        return 2.f*( extent.x*extent.z + extent.x*extent.y + extent.y*extent.z );
    }

    void expand(const glm::vec3 p) {
        bounds[0] = glm::min(bounds[0], p);
        bounds[1] = glm::max(bounds[1], p);
        extent = bounds[1] - bounds[0];
    }

    void expand(const AABB& bbox) {
        bounds[0] = glm::min(bounds[0], bbox.bounds[0]);
        bounds[1] = glm::max(bounds[1], bbox.bounds[1]);
        extent = bounds[1] - bounds[0];
    }

    glm::dvec3 centroid() {
        return {(bounds[1].x - bounds[0].x) / 2, (bounds[1].y - bounds[0].y) / 2, (bounds[1].z - bounds[0].z) / 2};
    }

    bool intersect(const Ray& ray, double& t0, double& t1) {
        float tmin, tmax, tymin, tymax, tzmin, tzmax;

        tmin = (bounds[ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
        tmax = (bounds[1-ray.sign[0]].x - ray.origin.x) * ray.invdir.x;
        tymin = (bounds[ray.sign[1]].y - ray.origin.y) * ray.invdir.y;
        tymax = (bounds[1-ray.sign[1]].y - ray.origin.y) * ray.invdir.y;

        if ((tmin > tymax) || (tymin > tmax))
            return false;
        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        tzmin = (bounds[ray.sign[2]].z - ray.origin.z) * ray.invdir.z;
        tzmax = (bounds[1-ray.sign[2]].z - ray.origin.z) * ray.invdir.z;

        if ((tmin > tzmax) || (tzmin > tmax))
            return false;
        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;

        if (tmin < t1 && tmax > t0) {
            t0 = tmin;
            t1 = tmax;
            return true;
        }
        return false;
    }

};
#endif //PPGSO_AABB_H
