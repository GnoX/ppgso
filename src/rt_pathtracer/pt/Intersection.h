#ifndef PPGSO_INTERSECTION_H
#define PPGSO_INTERSECTION_H

#include <glm/vec3.hpp>
#include <limits>
#include <cmath>
#include "Material.h"
#include "../defs.h"

/*!
 * Structure to represent a ray to object collision, the Intersection structure will contain material surface normal
 */
struct Intersection {
    double distance = INF;
    glm::vec3 position, normal;
    const Material *material;
};

static Intersection noHit{ INF, {0,0,0}, {0,0,0}, nullptr };

#endif //PPGSO_INTERSECTION_H
