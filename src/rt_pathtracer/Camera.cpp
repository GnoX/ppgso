
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

namespace pathtracer {

Camera::Camera(float fov, float ratio, float near, float far) {
    auto fov_rad = static_cast<float>((M_PI / 180.0f) * fov);
    projection = glm::perspective(fov_rad, ratio, near, far);
    update();
}

void Camera::update() {
    view = glm::lookAt(pos, pos + front, up);
}

void Camera::rotate(float xoffset, float yoffset) {
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    update_vectors();
}

void Camera::update_vectors() {
    front.x = static_cast<float>(cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
    front.y = static_cast<float>(sin(glm::radians(pitch)));
    front.z = static_cast<float>(sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, world_up));
    up    = glm::normalize(glm::cross(right, front));
}

void Camera::move(Camera::Direction dir) {

#define _case(dir, code) case dir: { code; } break;
    switch(dir) {
        _case(FRONT, pos += front * speed)
        _case(BACK, pos -= front * speed)
        _case(RIGHT, pos += right * speed)
        _case(LEFT, pos -= right * speed)
        _case(UP, pos += world_up * speed)
        _case(DOWN, pos -= world_up * speed)
    }
}

Ray Camera::generateRay(int x, int y, int width, int height) const {
    // Camera deltas
    glm::vec3 vdu = 2.0f * right / (float) width;
    glm::vec3 vdv = 2.0f * -up / (float) height;

    glm::vec3 direction = front
                           + vdu * (-width/2 + x + glm::linearRand(0.0f, 1.0f))
                           + vdv * (-height/2 + y + glm::linearRand(0.0f, 1.0f));
    Ray ray(pos, glm::normalize(direction));
    return ray;
}


}
