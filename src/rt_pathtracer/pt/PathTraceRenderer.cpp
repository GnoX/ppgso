
#include "PathTraceRenderer.h"
namespace pathtracer {
    namespace pt {
        PathTraceRenderer::PathTraceRenderer(unsigned width, unsigned height, unsigned num_threads,
                                             const std::string& environment_map_path)
                : texture{(int) width, (int) height}
                , sample_buffer(width, height)
                , width(width) , height(height)
                , num_threads(num_threads) {
            texture_shader.use();
            texture_shader.setUniform("Texture", texture);
            workers.resize(num_threads);
            if (!environment_map_path.empty()) {
                environment_map = HDRImage(environment_map_path);
            }
        }

        void PathTraceRenderer::render(bool updated) {
            if (updated) { restart(); }
            texture.update();
            quad.render();
        }

        void PathTraceRenderer::start() {
            unsigned tileSize = width / 32;
            workers_done = 0;
            for (unsigned y = 0; y < height; y += tileSize) {
                for (unsigned x = 0; x < width; x += tileSize) {
                    work_queue.put_work(Work(x, y, tileSize, tileSize));
                }
            }
            status = RENDERING;
        }

        void PathTraceRenderer::restart() {
            raytracing = false;
            current_sample = 1;
            for (auto worker : workers) {
                if (worker != nullptr) {
                    worker->join();
                    delete worker;
                }
            }
            work_queue.clear();

            sample_buffer.clear();
            texture.image.clear({0, 0, 0});
            texture.update();

            raytracing = true;
            start();

            for (int i = 0; i < num_threads; i++) {
                workers[i] = new std::thread(&PathTraceRenderer::worker_thread, this);
            }
        }

        void PathTraceRenderer::worker_thread() {
            Work work;
            while (raytracing) {
                if (work_queue.try_get_work(&work)) {
                    render_tile(work.tile_x, work.tile_y, work.tile_w, work.tile_h);
                } else {
                    status = SYNC_WORKERS;
                    workers_done++;
                    if (workers_done == num_threads) {
                        current_sample++;
                        start();
                        thread_sync_condition.notify_all();
                    } else {
                        std::unique_lock<std::mutex> lk(thread_sync_mutex);
                        thread_sync_condition.wait(lk, [this] { return !raytracing || status == RENDERING; });
                    }
                }
            }
        }

        void pathtracer::pt::PathTraceRenderer::render_tile(unsigned tile_x, unsigned tile_y,
                                                            unsigned tile_width, unsigned tile_height) {
            for (unsigned y = tile_y; y < tile_y + tile_height; y++) {
                for (unsigned x = tile_x; x < tile_x + tile_width; x++) {
                    if (!raytracing) return;


                    Spectrum color = sample_buffer.get_pixel(x, y);
                    auto ray = scene->camera.generateRay(x, y, width, height);
                    glm::vec3 focal_point = ray.point(focal_length);

                    for (unsigned i = 0; i < dof_complexity; i++) {
                        glm::vec3 disturbance = glm::linearRand(glm::vec3(.0f, .0f, .0),
                                                                glm::vec3(dispersion, dispersion,
                                                                          .0f));
                        glm::vec3 view_ray_origin = ray.origin + disturbance;
                        glm::vec3 direction = glm::normalize(focal_point - view_ray_origin);
                        color += trace_ray(Ray{view_ray_origin, direction}, 5) * (1 / dof_complexity);
                    }
                    sample_buffer.set_pixel(color, x, y);
                    color /= current_sample;
                    color.clamp(.0f, 1.0f);
                    texture.image.setPixel(x, y, color.r, color.g, color.b);
                }
            }
        }

        Spectrum PathTraceRenderer::trace_ray(const Ray &ray, unsigned int depth) const {
            if (depth == 0) return Spectrum{0, 0, 0};

            const Intersection hit = cast_ray(ray);

            // No hit
            if (hit.distance >= INF) {
                return sample_environment(ray);
            } else {
                // Emission
                Spectrum color = {hit.material->emission.r, hit.material->emission.g, hit.material->emission.b};

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
                        return  trace_ray(reflectedRay, depth - 1) + hit.material->emission;

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
                    color += trace_ray(diffuseRay, depth - 1) * hit.material->diffuse;
                }

                return color;
            }
        }

        Spectrum PathTraceRenderer::sample_environment(const Ray &ray) const {
            glm::dvec3 pt = ray.direction;
            double u = .5 + atan2(pt.z, pt.x) / (2 * M_PI);
            auto x = static_cast<unsigned int>(u * (environment_map.w - 1));
            double v = .5 - asin(pt.y) / M_PI;
            auto y = static_cast<unsigned int>(v * (environment_map.h - 1));
            return environment_map.get_pixel(x, y);
        }

//TODO: create sampler class for this
        glm::dvec3 PathTraceRenderer::CosineSampleHemisphere(const glm::dvec3 &normal) const {
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

        void PathTraceRenderer::stop() {
            raytracing = false;
            thread_sync_condition.notify_all();
            for (auto worker : workers) {
                if (worker != nullptr) {
                    worker->join();
                    delete worker;
                }
            }
        }

        Intersection PathTraceRenderer::cast_ray(const Ray &ray) const {
//            Intersection i = noHit;
//            for (auto &obj : scene->objects) {
//                auto lh = obj->intersect(ray);
//                if (lh.distance < i.distance) {
//                    i = lh;
//                }
//
//            }
//            return i;
            return bvh_accel->cast(ray);
        }

        void PathTraceRenderer::set_scene(Scene *scene) {
            this->scene = scene;
            bvh_accel = std::make_unique<BVH>(this->scene->objects, 5);
        }

    }
}
