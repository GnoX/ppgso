#include <ppgso/ppgso.h>
#include "Renderer.h"
#include "src/rt_pathtracer/gl/GLRenderer.h"
#include "src/rt_pathtracer/gl/Mesh.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
using namespace pathtracer;

class PathTracerWindow : public ppgso::Window {
public:
    PathTracerWindow() : Window{"rt_pathtracer", WINDOW_WIDTH, WINDOW_HEIGHT} {
        init_scene();
        disableCursor();
    }

    void init_scene() {
        scene = std::make_unique<Scene>();
        std::unique_ptr<gl::Mesh> mesh = std::make_unique<gl::Mesh>("cube.obj");

        scene->objects.emplace_back(std::move(mesh));
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
        gl_renderer.render(*scene);
    }

private:
    void update_scene() {
        scene->camera.rotate(static_cast<float>(scene->cursor.x - scene->prev_cursor.x),
                             static_cast<float>(scene->prev_cursor.y - scene->cursor.y));
        scene->prev_cursor = scene->cursor;

        if (scene->key[GLFW_KEY_W]) {
            scene->camera.move(pathtracer::Camera::Direction::FORWARD);
        }
        if (scene->key[GLFW_KEY_S]) {
            scene->camera.move(pathtracer::Camera::Direction::BACKWARD);
        }
        if (scene->key[GLFW_KEY_A]) {
            scene->camera.move(pathtracer::Camera::Direction::LEFT);
        }
        if (scene->key[GLFW_KEY_D]) {
            scene->camera.move(pathtracer::Camera::Direction::RIGHT);
        }
        if (scene->key[GLFW_KEY_SPACE]) {
            scene->camera.move(pathtracer::Camera::Direction::UP);
        }

        if (scene->key[GLFW_KEY_LEFT_SHIFT]) {
            scene->camera.move(pathtracer::Camera::Direction::DOWN);
        }


        scene->camera.update();
    }

    gl::GLRenderer gl_renderer;

    std::unique_ptr<Scene> scene;
};

int main() {
    PathTracerWindow window;
    while(window.pollEvents());
    return 0;
}
