#ifndef PPGSO_CAMERA_H
#define PPGSO_CAMERA_H


#include <glm/vec3.hpp>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <src/rt_pathtracer/pt/Ray.h>
#include <glm/gtc/random.hpp>

namespace pathtracer {

class Camera {
    glm::dvec3 up{0, 1, 0};
    glm::dvec3 front{0, 0, -1};
    glm::dvec3 right;
    glm::dvec3 world_up = up;
    glm::dvec3 pos{0, 0, 0};

    double yaw = -90.0f;
    double pitch = 0.0f;
    double mouse_sensitivity = .2f;
    double speed = .1f;

public:
    glm::mat4 projection;
    glm::mat4 view;

    Camera(float fov = 45.0f, float ratio = 1.0f, float near = 0.1f, float far = 100.0f);

    void update();

    enum Direction {
        FRONT, BACK, LEFT, RIGHT, UP, DOWN
    };

    void update_vectors();

    void move(Direction dir);

    void rotate(float xoffset, float yoffset);

    /*!
     * Generate a new Ray for the given viewport size and position
     * @param x Horizontal position in the viewport
     * @param y Vertical position in the viewport
     * @param width Width of the viewport
     * @param height Height of the viewport
     * @return Ray for the giver viewport position with small random deviation applied to support multi-sampling
     */
    Ray generateRay(int x, int y, int width, int height) const;

};

}

#endif //PPGSO_CAMERA_H

