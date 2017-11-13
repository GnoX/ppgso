#ifndef PPGSO_PATHTRACERENDERER_H
#define PPGSO_PATHTRACERENDERER_H


#include <src/rt_pathtracer/Renderer.h>
#include <shader.h>
#include <pt_shaders/texture_frag_glsl.h>
#include <pt_shaders/texture_vert_glsl.h>
#include <src/rt_pathtracer/gl/Mesh.h>
#include <thread>
#include <atomic>
#include <glm/gtx/compatibility.hpp>
#include "Ray.h"
#include "Intersection.h"


namespace pathtracer {
    namespace pt {
        class PathTraceRenderer : public Renderer {
        public:

            ppgso::Shader texture_shader{texture_vert_glsl, texture_frag_glsl};
            ppgso::Texture texture;
            std::vector<glm::vec3> sample_buffer;
            gl::Mesh quad{"quad.obj"};

            std::vector<std::thread*> workers;

            Scene *scene;
            int currentSample = 0;

            enum Status {
                RENDERING, IDLE, DONE
            } status = IDLE;

            int width;
            int height;
            std::atomic<bool> rendering;

            PathTraceRenderer(int width, int height)
                    : texture{width, height}
                    , sample_buffer(std::vector<glm::vec3>(width * height))
                    , width(width), height(height) {
                texture_shader.use();
                texture_shader.setUniform("Texture", texture);
                workers.resize(1);
            }

            void render(Scene &scene, bool updated) override {
                if (updated) {
                    this->scene = &scene;
                    restart();
                }
                texture.update();
                quad.render();
            }

            void stop() override {
                // TODO
            }

            void start() {
                status = RENDERING;
                rendering = true;

                workers[0] = new std::thread(&PathTraceRenderer::worker_thread, this);
            }

            void restart() {
                rendering = false;
                currentSample = 1;
                for (auto worker : workers) {
                    if (worker != nullptr) {
                        worker->join();
                        delete worker;
                    }
                }

                std::fill(sample_buffer.begin(), sample_buffer.end(), glm::vec3{0, 0, 0});
                texture.image.clear({0, 0, 0});
                texture.update();
                start();
            }

            void worker_thread() {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        if (rendering) {
                            glm::dvec3 color = sample_buffer[x + y * width];
                            auto ray = scene->camera.generateRay(x, y, width, height);
                            color += trace_ray(ray, 5);
                            sample_buffer[x + y * width] = color;

                            color = glm::clamp(color / (double) currentSample, 0.0, 1.0);
                            texture.image.setPixel(x, y, (float) color.r, (float) color.g, (float) color.b);
                        } else return;
                    }
                }
                currentSample++;
                status = IDLE;
            }

            inline Intersection cast(const Ray &ray) const {
                Intersection i = noHit;
                for (auto &obj : scene->objects) {
                    auto lh = obj->intersect(ray);
                    if (lh.distance < i.distance) {
                        i = lh;
                    }

                }
                return i;
            }

            /*
             * Trace a ray as it collides with objects in the world
             * @param ray Ray to trace
             * @param depth Maximum number of collisions to trace
             * @return Color representing the accumulated lighting for each ray collision
             */
            inline glm::vec3 trace_ray(const Ray &ray, unsigned int depth) const {
                glm::vec3 bg_color = {.0, .0, .0};
                if (depth == 0) return bg_color;

                const Intersection hit = cast(ray);

                // No hit
                if (std::isinf(hit.distance)) {
                    return {0, 0, 0};
                } else {
                    // Emission
                    glm::vec3 color = hit.material->emission;

                    // Decide to reflect or refract using linear random
                    if (hit.material->type == MaterialType::REFRACTIVE) {
                        const float refractionIndex = 1.5;

                        // Ideal specular reflection
                        glm::vec3 reflection = reflect(ray.direction, hit.normal);
                        // Ray of reflection
                        Ray reflectedRay{hit.position, reflection};

                        // Flip normal if the ray is "inside" a sphere
                        glm::vec3 normal = glm::dot(ray.direction, hit.normal) < 0 ? hit.normal : -hit.normal;
                        // Reverse the refraction index as well
                        float r_index = glm::dot(ray.direction, hit.normal) < 0 ? 1/refractionIndex : refractionIndex;

                        // Total internal refraction
                        float ddn = glm::dot(ray.direction, hit.normal);
                        float cos2t = 1-r_index*r_index*(1-ddn*ddn);
                        if(cos2t < 0)
                            return hit.material->emission + trace_ray(reflectedRay, depth - 1);

                        // Prepare refraction ray
                        glm::vec3 refraction = glm::refract(ray.direction, normal, r_index);
                        Ray refractionRay{hit.position, refraction};
                        // Trace the ray recursively
                        color += trace_ray(refractionRay, depth - 1);
                    }

                    if (hit.material->type == MaterialType::SPECULAR) {
                        // Ideal specular reflection
                        glm::vec3 reflection = glm::reflect(ray.direction, hit.normal);
                        // Ray of reflection
                        Ray reflectedRay{hit.position, reflection};
                        // Trace the ray recursively
                        color += trace_ray(reflectedRay, depth - 1);
                    }

                    if (hit.material->type == MaterialType::DIFFUSE) {
                        // Random diffuse reflection
                        glm::vec3 diffuse = CosineSampleHemisphere(hit.normal);
                        // Random diffuse ray
                        Ray diffuseRay{hit.position, diffuse};
                        // Trace the ray recursively
                        color += hit.material->diffuse * trace_ray(diffuseRay, depth - 1);
                    }

                    return color;
                }
            }

            inline glm::dvec3 CosineSampleHemisphere(const glm::dvec3 &normal) const {
                static const double SQRT_OF_ONE_THIRD = sqrt(1 / 3.0);
                double up = sqrt(glm::linearRand(0.0, 1.0)); // cos(theta)
                double over = sqrt(1 - up * up); // sin(theta)
                double around = glm::linearRand(0.0, 2.0) * M_PI;

                // Find a direction that is not the normal based off of whether or not the
                // normal's components are all equal to sqrt(1/3) or whether or not at
                // least one component is less than sqrt(1/3). Learned this trick from
                // Peter Kutz.

                glm::dvec3 directionNotNormal;
                if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
                    directionNotNormal = {1, 0, 0};
                } else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
                    directionNotNormal = {0, 1, 0};
                } else {
                    directionNotNormal = {0, 0, 1};
                }
                glm::dvec3 perpendicularDirection = glm::normalize(glm::cross(normal, directionNotNormal));

                return up * normal
                       + cos(around) * over * perpendicularDirection
                       + sin(around) * over * (glm::normalize(glm::cross(normal, perpendicularDirection)));
            }
        };

    }
}

#endif //PPGSO_PATHTRACERENDERER_H
