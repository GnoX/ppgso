#include <ppgso/ppgso.h>
#include <src/rt_pathtracer/pt/PathTraceRenderer.h>
#include "Renderer.h"
#include "src/rt_pathtracer/gl/GLRenderer.h"
#include "src/rt_pathtracer/gl/Mesh.h"
#include "Sphere.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
using namespace pathtracer;

class PathTracerWindow : public ppgso::Window {
public:
    bool scene_updated = true;
    PathTracerWindow() : Window{"rt_pathtracer", WINDOW_WIDTH, WINDOW_HEIGHT} {
        init_scene();
        disableCursor();
    }

    void init_scene() {
        scene = std::make_unique<Scene>();

        auto s1 = Sphere(10000, glm::vec3{0, -10010, -20}, Material::Light());
        auto s2 = Sphere(10000, glm::vec3{-10010, 0, -20}, Material::Blue());
        auto s3 = Sphere(10000, glm::vec3{10010, 0, -20}, Material::Red());
        auto s4 = Sphere(10000, glm::vec3{0, 0, -9090}, Material::Green());
        auto s5 = Sphere(10000, glm::vec3{0, 0, 10010}, Material::Cyan());
        scene->add(s1);
        scene->add(s2);
        scene->add(s3);
        scene->add(s4);
        scene->add(s5);
    }


    void onKey(int key, int scanCode, int action, int mods) override {
        scene->key[key] = action;
        scene->key_mod = mods;
    };

    void onCursorPos(double cursorX, double cursorY) override {
        scene->cursor.x = cursorX;
        scene->cursor.y = cursorY;
    }

    void onMouseButton(int button, int action, int mods) override {
        if(button == GLFW_MOUSE_BUTTON_LEFT) {
            scene->cursor.left = action == GLFW_PRESS;
        }
        if(button == GLFW_MOUSE_BUTTON_RIGHT) {
            scene->cursor.right = action == GLFW_PRESS;
        }
    }

    void onIdle() override {
        glClearColor(.3f, .3f, .3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        update_scene();
//        gl_renderer.render(*scene);
        pt_renderer.render(*scene, scene_updated);
    }

private:
    void update_scene() {
        scene_updated = false;
        auto dcx = static_cast<float>(scene->cursor.x - scene->prev_cursor.x);
        auto dcy = static_cast<float>(scene->prev_cursor.y - scene->cursor.y);
        if (dcx != 0 || dcy != 0 ) {
            scene->camera.rotate(dcx, dcy);
            scene_updated = true;
        }

        scene->prev_cursor = scene->cursor;

        if (scene->key[GLFW_KEY_W]) {
            scene->camera.move(pathtracer::Camera::Direction::FRONT);
            scene_updated = true;
        }
        if (scene->key[GLFW_KEY_S]) {
            scene->camera.move(pathtracer::Camera::Direction::BACK);
            scene_updated = true;
        }
        if (scene->key[GLFW_KEY_A]) {
            scene->camera.move(pathtracer::Camera::Direction::LEFT);
            scene_updated = true;
        }
        if (scene->key[GLFW_KEY_D]) {
            scene->camera.move(pathtracer::Camera::Direction::RIGHT);
            scene_updated = true;
        }
        if (scene->key[GLFW_KEY_SPACE]) {
            scene->camera.move(pathtracer::Camera::Direction::UP);
            scene_updated = true;
        }

        if (scene->key[GLFW_KEY_LEFT_SHIFT]) {
            scene->camera.move(pathtracer::Camera::Direction::DOWN);
            scene_updated = true;
        }

        scene->camera.update();
    }

    gl::GLRenderer gl_renderer;
    pt::PathTraceRenderer pt_renderer{WINDOW_WIDTH, WINDOW_HEIGHT};
    std::unique_ptr<Scene> scene;

};

int main() {
    PathTracerWindow window;
    while(window.pollEvents());
    return 0;
}
