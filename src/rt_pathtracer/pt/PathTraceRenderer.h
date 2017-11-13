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
#include <src/rt_pathtracer/HDRImage.h>
#include <src/rt_pathtracer/BVH.h>
#include "Ray.h"
#include "Intersection.h"


namespace pathtracer {
    namespace pt {

        struct Work {
            Work() : Work(0, 0, 0, 0) { }

            Work(unsigned x, unsigned y, unsigned w, unsigned h)
                    : tile_x(x), tile_y(y), tile_w(w), tile_h(h) {}

            unsigned tile_x;
            unsigned tile_y;
            unsigned tile_w;
            unsigned tile_h;
        };

        class PathTraceRenderer : public Renderer {
        public:
            PathTraceRenderer(unsigned width, unsigned height, unsigned num_threads = 4,
                              const std::string& environment_map_path = "");
            void render(bool updated) override;
            void stop() override;

            void set_scene(Scene *scene) override;

        private:
            ppgso::Shader texture_shader{texture_vert_glsl, texture_frag_glsl};
            ppgso::Texture texture;
            HDRImage sample_buffer;
            gl::Mesh quad{"quad.obj"};
            std::vector<std::thread*> workers;
            Scene *scene;
            HDRImage environment_map;
            std::unique_ptr<BVH> bvh_accel;

            unsigned current_sample = 0;
            unsigned num_threads;
            unsigned width;
            unsigned height;

            std::atomic<bool> raytracing;
            std::atomic<int> workers_done;

            std::mutex thread_sync_mutex;
            std::condition_variable thread_sync_condition;
            WorkQueue<Work> work_queue;

            enum Status {
                RENDERING, IDLE, SYNC_WORKERS, DONE
            } status = IDLE;

            void start();

            void restart();

            void worker_thread();

            void render_tile(unsigned tile_x, unsigned tile_y, unsigned tile_width, unsigned tile_height);

            inline Intersection cast(const Ray &ray) const;

            inline Spectrum trace_ray(const Ray &ray, unsigned int depth) const;

            Spectrum sample_environment(const Ray &ray) const;

            inline glm::dvec3 CosineSampleHemisphere(const glm::dvec3 &normal) const;

        };

    }
}

#endif //PPGSO_PATHTRACERENDERER_H
