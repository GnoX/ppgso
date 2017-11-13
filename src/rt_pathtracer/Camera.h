#ifndef PPGSO_CAMERA_H
#define PPGSO_CAMERA_H


#include <glm/vec3.hpp>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>

namespace pathtracer {

class Camera {
    glm::vec3 up{0, 1, 0};
    glm::vec3 front{0, 0, -1};
    glm::vec3 right;
    glm::vec3 world_up = up;
    glm::vec3 pos{0, 0, 0};

    float yaw = -90.0f;
    float pitch = 0.0f;
    float mouse_sensitivity = .2f;
    float speed = .1f;

public:
    Camera(float fov = 45.0f, float ratio = 1.0f, float near = 0.1f, float far = 100.0f);

    void update();

    enum Direction {
        FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
    };

    void update_vectors();

    void move(Direction dir);

    void rotate(float xoffset, float yoffset);

    glm::mat4 projection;
    glm::mat4 view;
};

}

#endif //PPGSO_CAMERA_H

