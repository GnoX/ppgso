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
#include <src/rt_pathtracer/WorkQueue.h>
#include <condition_variable>
#include "Ray.h"
#include "Intersection.h"


namespace pathtracer {
    namespace pt {

        struct Work {
            Work() : Work(0, 0, 0, 0) { }

            Work(int x, int y, int w, int h)
                    : tile_x(x), tile_y(y), tile_w(w), tile_h(h) {}

            int tile_x;
            int tile_y;
            int tile_w;
            int tile_h;
        };

        class PathTraceRenderer : public Renderer {
        public:
            ppgso::Shader texture_shader{texture_vert_glsl, texture_frag_glsl};
            ppgso::Texture texture;
            std::vector<glm::vec3> sample_buffer;
            gl::Mesh quad{"quad.obj"};

            std::vector<std::thread*> workers;

            Scene *scene;
            int currentSample = 0;
            int num_threads;

            enum Status {
                RENDERING, IDLE, SYNC_WORKERS, DONE
            } status = IDLE;

            int width;
            int height;
            std::atomic<bool> raytracing;
            std::atomic<int> workers_done;
            std::mutex thread_sync_mutex;
            std::condition_variable thread_sync_condition;

            WorkQueue<Work> work_queue;

            PathTraceRenderer(int width, int height, int num_threads = 8)
                    : texture{width, height}
                    , sample_buffer(std::vector<glm::vec3>(width * height))
                    , width(width) , height(height)
                    , num_threads(num_threads){
                texture_shader.use();
                texture_shader.setUniform("Texture", texture);
                workers.resize(num_threads);
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
                raytracing = false;
                for (auto worker : workers) {
                    if (worker != nullptr) {
                        worker->join();
                        delete worker;
                    }
                }
            }

            void start() {
                int tileSize = width / 32;
                workers_done = 0;
                for (int y = 0; y < height; y += tileSize) {
                    for (int x = 0; x < width; x += tileSize) {
                        work_queue.put_work(Work(x, y, tileSize, tileSize));
                    }
                }
                status = RENDERING;
            }

            void restart() {
                raytracing = false;
                currentSample = 1;
                for (auto worker : workers) {
                    if (worker != nullptr) {
                        worker->join();
                        delete worker;
                    }
                }
                work_queue.clear();

                std::fill(sample_buffer.begin(), sample_buffer.end(), glm::vec3{0, 0, 0});
                texture.image.clear({0, 0, 0});
                texture.update();

                raytracing = true;
                start();

                for (int i = 0; i < num_threads; i++) {
                    workers[i] = new std::thread(&PathTraceRenderer::worker_thread, this);
                }
            }

            void worker_thread() {
                Work work;
                while (raytracing) {
                    if (work_queue.try_get_work(&work)) {
                        render_tile(work.tile_x, work.tile_y, work.tile_w, work.tile_h);
                    } else {
                        status = SYNC_WORKERS;
                        workers_done++;
                        if (workers_done == num_threads) {
                            currentSample++;
                            start();
                            thread_sync_condition.notify_all();
                        } else {
                            std::unique_lock<std::mutex> lk(thread_sync_mutex);
                            thread_sync_condition.wait(lk, [this] { return status == RENDERING; });
                        }
                    }
                }
            }

            void render_tile(int tile_x, int tile_y, int tile_width, int tile_height) {
                for (int y = tile_y; y < tile_y + tile_height; y++) {
                    for (int x = tile_x; x < tile_x + tile_width; x++) {
                        if (!raytracing) return;
                        glm::dvec3 color = sample_buffer[x + y * width];
                        auto ray = scene->camera.generateRay(x, y, width, height);
                        color += trace_ray(ray, 5);
                        sample_buffer[x + y * width] = color;

                        color = glm::clamp(color / (double) currentSample, 0.0, 1.0);
                        texture.image.setPixel(x, y, (float) color.r, (float) color.g, (float) color.b);
                    }
                }
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
