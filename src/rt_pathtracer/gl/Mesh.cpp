#include <GL/glew.h>
#include <tiny_obj_loader.h>
#include <sstream>
#include "Mesh.h"

namespace pathtracer {
    namespace gl {

        Mesh::Mesh(std::string obj_file) {
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            shapes.clear();
            materials.clear();
            std::string err = tinyobj::LoadObj(shapes, materials, obj_file.c_str());

            if (!err.empty()) {
                std::stringstream msg;
                msg << err << std::endl << "Failed to load OBJ file " << obj_file << "!" << std::endl;
                throw std::runtime_error(msg.str());
            }

            // Initialize OpenGL Buffers
            for(auto& shape : shapes) {
                gl_buffer buffer;

                if(!shape.mesh.positions.empty()) {
                    // Generate a vertex array object
                    glGenVertexArrays(1, &buffer.vao);
                    glBindVertexArray(buffer.vao);

                    // Generate and upload a buffer with vertex positions to GPU
                    glGenBuffers(1, &buffer.vbo);
                    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
                    glBufferData(GL_ARRAY_BUFFER, shape.mesh.positions.size() * sizeof(float), shape.mesh.positions.data(),
                                 GL_STATIC_DRAW);

                    // Bind the buffer to "Position" attribute in program
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                }

                if(!shape.mesh.texcoords.empty()) {
                    // Generate and upload a buffer with texture coordinates to GPU
                    glGenBuffers(1, &buffer.tbo);
                    glBindBuffer(GL_ARRAY_BUFFER, buffer.tbo);
                    glBufferData(GL_ARRAY_BUFFER, shape.mesh.texcoords.size() * sizeof(float), shape.mesh.texcoords.data(),
                                 GL_STATIC_DRAW);

                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
                }

                if(!shape.mesh.normals.empty()) {
                    // Generate and upload a buffer with texture coordinates to GPU
                    glGenBuffers(1, &buffer.nbo);
                    glBindBuffer(GL_ARRAY_BUFFER, buffer.nbo);
                    glBufferData(GL_ARRAY_BUFFER, shape.mesh.normals.size() * sizeof(float), shape.mesh.normals.data(),
                                 GL_STATIC_DRAW);

                    glEnableVertexAttribArray(2);
                    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                }

                // Generate and upload a buffer with indices to GPU
                glGenBuffers(1, &buffer.ibo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.mesh.indices.size() * sizeof(unsigned int), shape.mesh.indices.data(), GL_STATIC_DRAW);
                buffer.size = (GLsizei) shape.mesh.indices.size();

                // Copy it to the end of the buffers vector
                buffers.push_back(buffer);
            }

        }

        void Mesh::render() const {
            for(auto& buffer : this->buffers) {
                // Draw object
                glBindVertexArray(buffer.vao);
                glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);
            }
        }
    }
}
