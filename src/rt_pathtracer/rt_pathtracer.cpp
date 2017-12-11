#define STB_IMAGE_IMPLEMENTATION
#include <ppgso/ppgso.h>
#include <src/rt_pathtracer/pt/PathTraceRenderer.h>
#include "src/rt_pathtracer/gl/GLRenderer.h"
#include "src/rt_pathtracer/pt/Triangle.h"
#include "Sphere.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
using namespace pathtracer;


struct Vertex {
    glm::vec4 position;
//    glm::vec4 normal;
//    glm::vec2 texCoord;
    glm::vec4 color;
};

struct Face {
    Vertex v0, v1, v2;
};

std::vector<Face> load_obj(const std::string filename) {
    // Using tiny obj loader from ppgso lib
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err = tinyobj::LoadObj(shapes, materials, filename.c_str());

    // Will only convert 1st shape to Faces
    auto &mesh = shapes[0].mesh;

    // Collect data in vectors
    std::vector<glm::vec4> positions;
    for (int i = 0; i < (int) mesh.positions.size() / 3; ++i)
        positions.emplace_back(mesh.positions[3 * i], mesh.positions[3 * i + 1], mesh.positions[3 * i + 2], 1);

    std::vector<glm::vec4> normals;
    for (int i = 0; i < (int) mesh.normals.size() / 3; ++i)
        normals.emplace_back(mesh.normals[3 * i], mesh.normals[3 * i + 1], mesh.normals[3 * i + 2], 1);

    std::vector<glm::vec2> texcoords;
    for (int i = 0; i < (int) mesh.texcoords.size() / 2; ++i)
        texcoords.emplace_back(mesh.texcoords[2 * i], mesh.texcoords[2 * i + 1]);

    // Fill the vector of Faces with data
    std::vector<Face> faces(mesh.indices.size() / 3);
    for (int i = 0; i < (int) faces.size(); i++) {
        faces[i] = Face{
                {
                        positions[mesh.indices[i * 3]],
//                        normals[mesh.indices[i * 3]],
//                        texcoords[mesh.indices[i * 3]],
                        {1, 1, 1, 1}
                }, {
                        positions[mesh.indices[i * 3 + 1]],
//                        normals[mesh.indices[i * 3 + 1]],
//                        texcoords[mesh.indices[i * 3 + 1]],
                        {1, 1, 1, 1}
                }, {
                        positions[mesh.indices[i * 3 + 2]],
//                        normals[mesh.indices[i * 3 + 2]],
//                        texcoords[mesh.indices[i * 3 + 2]],
                        {1, 1, 1, 1}
                }
        };
    }
    return faces;
}


class PathTracerWindow : public ppgso::Window {
public:
    bool scene_updated = true;
    bool cursor_disabled = false;
    PathTracerWindow() : Window{"rt_pathtracer", WINDOW_WIDTH, WINDOW_HEIGHT} {
        init_scene();
        showCursor();
    }

    void init_scene() {
        scene = std::make_unique<Scene>();

        int size = 5;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                auto sphere = Sphere(1, glm::vec3(0, i * 2.1f, j * 2.1f),
                                     Material::new_mr_material(i / (float) size, j / (float) size, {1.0f, 1.0f, 1.0f}));
                scene->add(sphere);
            }
        }
        auto faces = load_obj("bunny.obj");
        Material m = Material::new_mr_material(0.0f, 1.0f, {1.0f, 0.0f, 0.0f});
        glm::mat4 translation = glm::translate(glm::mat4(), glm::vec3(1, -13, -3.0));
        glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(90.0));
        glm::mat4 model = translation * scale;
        for (auto &face : faces) {
            auto v0 = model * face.v0.position;
            auto v1 = model * face.v1.position;
            auto v2 = model * face.v2.position;
            glm::vec3 vv0(v0.x, v0.y, v0.z);
            glm::vec3 vv1(v1.x, v1.y, v1.z);
            glm::vec3 vv2(v2.x, v2.y, v2.z);
            auto triangle = pt::Triangle(vv0, vv1, vv2, m);
            scene->add(triangle);
//            objects.emplace_back(std::move(std::make_unique<pt::Triangle>(vv0, vv1, vv2, m)));
        }


//        auto s1 = Sphere(1, glm::vec3{-3, 0, -3}, Material::Bricks());
//////        auto s2 = Sphere(1, glm::vec3{0, 0, 5.7}, Material::ReflectiveAndRefractive());
////        auto s3 = Sphere(1, glm::vec3{0, 0, -8.4}, Material::Mirror());
//        auto s2 = Sphere(1, glm::vec3{0, 0, -3}, Material::MetalScuffs());
////        auto s1 = Sphere(1, glm::vec3(0, 0, -3), Material::new_mr_material(1.0f, 0.50f, {0.0f, 0.0f, 1.0f}));
//        scene->add(s1);
//        scene->add(s2);
//        scene->add(s3);
//        scene->add(s4);
        pt_renderer.set_scene(scene.get());
        gl_renderer.set_scene(scene.get());
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
        if (glfwWindowShouldClose(window)) {
            pt_renderer.stop();
        }
        glClearColor(.3f, .3f, .3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        update_scene();
//        gl_renderer.render(*scene);
        pt_renderer.render(scene_updated);
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

        if (scene->last_key[GLFW_KEY_F3] && !scene->key[GLFW_KEY_F3]) {
            cursor_disabled = !cursor_disabled;
            if (cursor_disabled) {
                disableCursor();
            } else {
                showCursor();
            }

        }
        scene->last_key = scene->key;


        scene->camera.update();
    }

    gl::GLRenderer gl_renderer;
//    pt::PathTraceRenderer pt_renderer{WINDOW_WIDTH, WINDOW_HEIGHT, 4, "env_map_2.hdr"};
    pt::PathTraceRenderer pt_renderer{WINDOW_WIDTH, WINDOW_HEIGHT, 4, "environment_map.hdr"};
//    pt::PathTraceRenderer pt_renderer{WINDOW_WIDTH, WINDOW_HEIGHT, 1, "debug_env.jpg"};
    std::unique_ptr<Scene> scene;

};

int main() {
    PathTracerWindow window;
    while(window.pollEvents());
    return 0;
}
