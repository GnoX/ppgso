#ifndef PPGSO_RAY_H
#define PPGSO_RAY_H

#include <glm/vec3.hpp>

/*!
 * Structure holding origin and direction that represents a ray
 */
struct Ray {
    glm::vec3 origin, direction;
    glm::vec3 invdir;
    int sign[3];

    Ray(const glm::vec3 &origin, const glm::vec3 &direction) : origin(origin), direction(direction) {
        invdir = glm::vec3(1 / direction.x, 1 / direction.y, 1 / direction.z);
        sign[0] = (invdir.x < 0);
        sign[1] = (invdir.y < 0);
        sign[2] = (invdir.z < 0);
    }

    /*!
     * Compute a point on the ray
     * @param t Distance from origin
     * @return Point on ray where t is the distance from the origin
     */
    inline glm::vec3 point(float t) const {
        return origin + direction * t;
    }
};


#endif //PPGSO_RAY_H
