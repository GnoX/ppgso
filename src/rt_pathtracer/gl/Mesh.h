#ifndef PPGSO_MESH_H
#define PPGSO_MESH_H

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <GL/gl.h>
#include <src/rt_pathtracer/Object.h>
#include <string>

namespace pathtracer { namespace gl {

    class Mesh : public Object {

        struct gl_buffer {
        public:
            GLuint vao, vbo, tbo, nbo, ibo = 0;
            GLsizei size = 0;
        };

        std::vector<gl_buffer> buffers;

    public:

        Mesh(std::string gltf_file);

        void render() const override;
    };
}}


#endif //PPGSO_MESH_H
