
#include "PathTraceRenderer.h"

#include <random>

namespace pathtracer { namespace pt {
static auto rng = std::mt19937(std::random_device()());

PathTraceRenderer::PathTraceRenderer(unsigned width, unsigned height, unsigned num_threads,
                                     const std::string& environment_map_path)
        : texture{(int) width, (int) height}
        , sample_buffer(width, height)
        , width(width) , height(height)
        , num_threads(num_threads)
        , tileSize(width / 128)
        , work_blocks(std::vector<Work>((width * height) / (tileSize * tileSize))) {
    texture_shader.use();
    texture_shader.setUniform("Texture", texture);
    workers.resize(num_threads);
    prepare_work();

    if (!environment_map_path.empty()) {
        environment_map = HDRImage3(environment_map_path);
    }
}


void PathTraceRenderer::render(bool updated) {
    if (updated) { restart(); }
    texture.update();
    quad.render();
}

void PathTraceRenderer::start() {
    workers_done = 0;
    std::shuffle(work_blocks.begin(), work_blocks.end(), rng);
    for (auto work : work_blocks) {
        work_queue.put_work(work);
    }
    status = RENDERING;
}

void PathTraceRenderer::prepare_work() {
    unsigned i = 0;
    for (unsigned y = 0; y < height; y += tileSize) {
        for (unsigned x = 0; x < width; x += tileSize) {
            work_blocks[i++] = Work(x, y, tileSize, tileSize);
        }
    }
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
            //TODO: there's a deadlock in here
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


            Spectrum3 color = sample_buffer.get_pixel(x, y);
            auto ray = scene->camera.generateRay(x, y, width, height);

            if (dof_complexity > 0) {
                glm::vec3 focal_point = ray.point(focal_length);
                for (unsigned i = 0; i < dof_complexity; i++) {
                    glm::vec3 disturbance = glm::linearRand(glm::vec3(.0f, .0f, .0),
                                                            glm::vec3(dispersion, dispersion,
                                                                      .0f));
                    glm::vec3 view_ray_origin = ray.origin + disturbance;
                    glm::vec3 direction = glm::normalize(focal_point - view_ray_origin);
                    color += trace_ray(Ray{view_ray_origin, direction}, 5) * (1 / dof_complexity);
                }
            } else {
                color += trace_ray(ray, 5);
            }
            sample_buffer.set_pixel(color, x, y);
            color /= current_sample;
            color.clamp(.0f, 1.0f);
            texture.image.setPixel(x, y, color.c[0], color.c[1], color.c[2]);
        }
    }
}

float ggx_geometric_occlusion(float NoL, float NoV, float r) {
    float attenuationL = 2.0f * NoL / (NoL + glm::sqrt(r * r + (1.0f - r * r) * (NoL * NoL)));
    float attenuationV = 2.0f * NoV / (NoV + glm::sqrt(r * r + (1.0f - r * r) * (NoV * NoV)));
    return attenuationL * attenuationV;
}

float fresnel(const glm::vec3 &I, const glm::vec3 &N, const float &ior) {
    float kr;
    float cosi = glm::clamp(-1.0f, 1.0f, glm::dot(I, N));
    float etai = 1, etat = ior;
    if (cosi > 0) { std::swap(etai, etat); }
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    if (sint >= 1) {
        kr = 1;
    } else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
    return kr;
}

glm::vec3 ggx_importance_sample(float roughness, glm::vec3 n) {
    glm::vec2 xi = glm::linearRand(glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 1.0f});
    float alpha = roughness * roughness;
    float phi = 2 * (float) M_PI * xi.x;
    float cos_theta = glm::sqrt((1 - xi.y) / (1 + (alpha * alpha - 1) * xi.y));
    float sin_theta = glm::sqrt(1 - cos_theta * cos_theta);

    glm::vec3 h = {sin_theta * glm::cos(phi),
                   sin_theta * glm::sin(phi),
                   cos_theta};

    glm::vec3 up = (glm::abs(n.z) < .999) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 tangent_x = glm::normalize(glm::cross(up, n));
    glm::vec3 tangent_y = glm::cross(n, tangent_x);
    return tangent_x * h.x + tangent_y * h.y + n * h.z;
}

Spectrum3 PathTraceRenderer::trace_ray(const Ray &ray, unsigned int depth) const {
    if (depth == 0) return Spectrum3{0, 0, 0};

    const Intersection hit = cast_ray(ray);

    // No hit
    if (hit.distance >= INF) {
        return sample_environment(ray);
    } else {
        Spectrum3 color = {hit.material->emission.r, hit.material->emission.g, hit.material->emission.b};
        auto m = hit.material;
        auto n = hit.normal;

        if (m->type == MaterialType::METALLIC_ROUGHNESS) {
            auto metalness = m->get_metalness(hit.uv)[0];
            if (glm::linearRand(0.0f, 1.0f) < metalness) {
                glm::vec3 v = -ray.direction;
                float roughness = m->get_roughness(hit.uv)[0];
                glm::vec3 half_vector = ggx_importance_sample(roughness, n);
                glm::vec3 l = 2.0f * glm::max(glm::dot(v, half_vector), 0.0f) * half_vector - v;

                float NoV = glm::max(glm::dot(n, v), 0.0f);
                float NoL = glm::max(glm::dot(n, l), 0.0f);
                float NoH = glm::max(glm::dot(n, half_vector), 0.0f);
                float VoH = glm::max(glm::dot(v, half_vector), 0.0f);

                float G = ggx_geometric_occlusion(NoL, NoV, roughness);
                float Fc = glm::pow(1.0f - VoH, 5.0f);
                glm::vec3 F = (1 - Fc) * m->specular_color + Fc;
                Ray next = Ray{hit.position, l};
                color += trace_ray(next, depth - 1)
                         * F * G * VoH / ((NoH * NoV) + .05);
            } else {
                float reflection = fresnel(ray.direction, n, m->ior);
                if (glm::linearRand(0.0f, 1.0f) < reflection) {
                    glm::vec3 reflected = glm::reflect(ray.direction, hit.normal);
                    Ray reflectedRay{hit.position, reflected};
                    color += trace_ray(reflectedRay, depth - 1); // * m->specular_color;
                } else {
                    float transparency = m->transparency;
                    if (glm::linearRand(0.0f, 1.0f) < transparency) {
                        float cosa = glm::dot(hit.position, n);
                        glm::vec3 normal = cosa < 0 ? n : -n;
                        float ior = cosa < 0 ? 1 / m->ior : m->ior;
                        glm::vec3 refraction = glm::refract(ray.direction, n, ior);
                        Ray refractionRay{hit.position - normal * (float) DELTA, refraction};
                        color += trace_ray(refractionRay, depth - 1);
                    } else {
                        glm::vec3 albedo = m->get_albedo(hit.uv).to_vec3();
                        glm::vec3 diffuse = CosineSampleHemisphere(hit.normal);
                        Ray diffuseRay{hit.position, diffuse};
                        color += trace_ray(diffuseRay, depth - 1) * albedo;
                    }
                }
            }
        }


        return color;
    }
}

Spectrum3 PathTraceRenderer::sample_environment(const Ray &ray) const {
    glm::dvec3 pt = ray.direction;
    double u = .5 + atan2(pt.z, pt.x) / (2 * M_PI);
    auto x = static_cast<unsigned int>(u * (environment_map.w - 1));
    double v = .5 - asin(pt.y) / M_PI;
    auto y = static_cast<unsigned int>(v * (environment_map.h - 1));
    return environment_map.get_pixel(x, y);
}

//TODO: create sampler class for this
glm::vec3 PathTraceRenderer::CosineSampleHemisphere(const glm::vec3 &normal) const {
    static const double SQRT_OF_ONE_THIRD = sqrt(1 / 3.0);
    float up = std::sqrt(glm::linearRand(0.0f, 1.0f)); // cos(theta)
    float over = std::sqrt(1 - up * up); // sin(theta)
    auto around = static_cast<float>(glm::linearRand(0.0f, 2.0f) * M_PI);

    // Find a direction that is not the normal based off of whether or not the
    // normal's components are all equal to sqrt(1/3) or whether or not at
    // least one component is less than sqrt(1/3). Learned this trick from
    // Peter Kutz.

    glm::vec3 directionNotNormal;
    if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
        directionNotNormal = {1, 0, 0};
    } else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
        directionNotNormal = {0, 1, 0};
    } else {
        directionNotNormal = {0, 0, 1};
    }
    glm::vec3 perpendicularDirection = glm::normalize(glm::cross(normal, directionNotNormal));

    return up * normal
           + std::cos(around) * over * perpendicularDirection
           + std::sin(around) * over * (glm::normalize(glm::cross(normal, perpendicularDirection)));
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

}}
