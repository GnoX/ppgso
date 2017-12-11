#ifndef PPGSO_TRIANGLE_H
#define PPGSO_TRIANGLE_H


#include <glm/vec3.hpp>
#include <utility>
#include <src/rt_pathtracer/TraceableObject.h>
#include "src/rt_pathtracer/gl/Mesh.h"

namespace pathtracer { namespace pt {
    class Triangle : public TraceableObject {
    public:
        const Material material;

        Triangle(pathtracer::gl::Mesh *mesh, uint32_t v0, uint32_t v1, uint32_t v2);
        Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Material material)
                : material(std::move(material)), v0(v0), v1(v1), v2(v2) {}

        void render() const override;

        Intersection intersect(const Ray &ray) const override;

        AABB get_bbox() const override;
        glm::uvec2 get_uv(glm::vec3 point) const;

//        pathtracer::gl::Mesh *mesh;
        glm::vec3 v0, v1, v2;

//        uint32_t v0, v1, v2;
//        std::shared_ptr<Material> material;

    };
}}


#endif //PPGSO_TRIANGLE_H
