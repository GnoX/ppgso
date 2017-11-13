#include <ppgso/ppgso.h>
#include "Renderer.h"
#include "Scene.h"
#include "src/rt_pathtracer/gl/GLRenderer.h"
#include "src/rt_pathtracer/gl/Mesh.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
using namespace pathtracer;

class PathTracerWindow : public ppgso::Window {
public:
    PathTracerWindow() : Window{"rt_pathtracer", WINDOW_WIDTH, WINDOW_HEIGHT} {
        init_scene();
    }

    void init_scene() {
        scene = std::make_unique<Scene>();
        std::unique_ptr<gl::Mesh> mesh = std::make_unique<gl::Mesh>("quad.obj");

        scene->objects.emplace_back(std::move(mesh));
    }

    void onIdle() override {
        gl_renderer.render(*scene);
    }
private:
    gl::GLRenderer gl_renderer;

    std::unique_ptr<Scene> scene;
};

int main() {
    PathTracerWindow window;
    while(window.pollEvents());
    return 0;
}
