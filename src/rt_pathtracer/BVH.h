
#ifndef PPGSO_BVH_H
#define PPGSO_BVH_H

#include <ppgso/ppgso.h>
#include <algorithm>
#include "AABB.h"
#include "TracableObject.h"

struct BVHNode {
    AABB bbox;
    size_t n_start, n_primitives;
    std::shared_ptr<BVHNode> left;
    std::shared_ptr<BVHNode> right;

    BVHNode(AABB bbox, size_t n_start, size_t n_primitives) : bbox(bbox)
            , n_start(n_start)
            , n_primitives(n_primitives) {}
    bool is_leaf() {
        return left.use_count() == 0 && right.use_count() == 0;
    }
};

#define NUM_BUCKETS 12

class BVH {
public:
    std::unique_ptr<BVHNode> root;
    std::vector<TracableObject*> primitives;

    BVH(std::vector<std::unique_ptr<TracableObject>>& primitives, size_t leaf_size) {

        this->primitives = std::vector<TracableObject*>(primitives.size(), nullptr);
        for (unsigned i = 0; i < this->primitives.size(); i++) {
            this->primitives[i] = primitives[i].get();
        }

        AABB bbox = this->primitives[0]->get_bbox();
        for (size_t i = 1; i < this->primitives.size(); ++i) {
            bbox.expand(this->primitives[i]->get_bbox());
        }

        root = std::make_unique<BVHNode>(bbox, (size_t) 0, this->primitives.size());
        build(root.get(), leaf_size);
    }


    static inline int compute_bucket(double centroid_min, double centroid_max, double centroid)
    {
        int result = (int) glm::floor((centroid - centroid_min) / (centroid_max - centroid_min + EPS) * (NUM_BUCKETS - 1));
        assert(0 <= result && result < NUM_BUCKETS);
        return result;
    }

    void build(BVHNode* node, size_t leaf_size) {
        if (node->n_primitives <= leaf_size) return;
        double csah = 1.0 / node->bbox.surface_area();

        double best_sah = INF;
        AABB best_left_bbox, best_right_bbox;
        size_t best_left_count, best_right_count;
        int best_axis;
        int best_pplane;
        double best_cmax, best_cmin;

        for (int axis = 0; axis < 3; ++axis) {
            AABB bboxes[NUM_BUCKETS] = {AABB()};
            int prim_counts[NUM_BUCKETS] = {0};

            double centroid_min = INF;
            double centroid_max = -INF;
            for (int i = node->n_start; i < node->n_start + node->n_primitives; ++i) {
                double centroid = primitives[i]->get_bbox().centroid()[axis];
                if (centroid < centroid_min)
                    centroid_min = centroid;
                if (centroid > centroid_max)
                    centroid_max = centroid;
            }

            if (centroid_max - centroid_min < EPS) continue;

            for (int i = node->n_start; i < node->n_start + node->n_primitives; ++i) {
                AABB bbox = primitives[i]->get_bbox();
                double centroid = bbox.centroid()[axis];
                int bucket = compute_bucket(centroid_min, centroid_max, centroid);
                bboxes[bucket].expand(bbox);
                ++prim_counts[bucket];
            }

            for (int pplane = 1; pplane < NUM_BUCKETS; ++pplane) {
                AABB left_bbox, right_bbox;
                int left_count = 0, right_count = 0;

                for (int j = 0; j < pplane; ++j) {
                    left_bbox.expand(bboxes[j]);
                    left_count += prim_counts[j];
                }

                for (int j = pplane; j < NUM_BUCKETS; ++j) {
                    right_bbox.expand(bboxes[j]);
                    right_count += prim_counts[j];
                }

                if (left_count == 0 || right_count == 0) continue;

                double sah = left_bbox.surface_area() * csah * left_count +
                             right_bbox.surface_area() * csah * right_count;

                if (sah < best_sah) {
                    best_sah = sah;
                    best_left_bbox = left_bbox;
                    best_right_bbox = right_bbox;
                    best_left_count = left_count;
                    best_right_count = right_count;
                    best_axis = axis;
                    best_pplane = pplane;
                    best_cmax = centroid_max;
                    best_cmin = centroid_min;
                }
            }
        }

        if (best_sah == INF) {
            AABB bbox_left, bbox_right;
            for (size_t j = node->n_start; j < node->n_start + node->n_primitives / 2; j++) {
                bbox_left.expand(primitives[j]->get_bbox());
            }
            for (size_t j = node->n_start + node->n_primitives / 2; j < node->n_start + node->n_primitives; j++) {
                bbox_right.expand(primitives[j]->get_bbox());
            }
            node->left = std::make_shared<BVHNode>(bbox_left, node->n_start, node->n_primitives / 2);
            node->right = std::make_shared<BVHNode>(bbox_right, node->n_start + node->n_primitives / 2, node->n_primitives / 2);
        } else {
            auto prim_in_left =
                    [&best_axis, &best_pplane, &best_cmin, &best_cmax](const TracableObject* obj) {
                        double centroid = obj->get_bbox().centroid()[best_axis];
                        return compute_bucket(best_cmin, best_cmax, centroid) < best_pplane;
                    };

            std::partition(primitives.begin() + node->n_start,
                           primitives.begin() + node->n_start + node->n_primitives,
                           prim_in_left);

            node->left = std::make_shared<BVHNode>(best_left_bbox, node->n_start, best_left_count);
            node->right = std::make_shared<BVHNode>(best_right_bbox, node->n_start + best_left_count, best_right_count);
        }


        build(node->left.get(), leaf_size);
        build(node->right.get(), leaf_size);
    }

    Intersection cast(const Ray &ray) const {
        Intersection hit = noHit;
        traverse(ray, root.get(), hit);
        return hit;
    }

private:
    void traverse(const Ray& ray, BVHNode *node, Intersection& hit) const {
        double t0 = -INF, t1 = INF;
        if (!node->bbox.intersect(ray, t0, t1) || hit.distance < t0) return;

        if (node->is_leaf()) {
            for (size_t i = node->n_start; i < node->n_start + node->n_primitives; i++) {
                Intersection lh = primitives[i]->intersect(ray);
                if (!std::isinf(lh.distance)) {
                    if (lh.distance > 0 && lh.distance < hit.distance) {
                        hit = lh;
                    }
                }
            }
        } else {
            double tl0 = -INF, tl1 = INF;
            double tr0 = -INF, tr1 = INF;
            node->bbox.intersect(ray, tl0, tl1);
            node->bbox.intersect(ray, tr0, tr1);

            std::shared_ptr<BVHNode> first = (tl0 <= tr0) ? node->left : node->right;
            std::shared_ptr<BVHNode> second = (tl0 <= tr0) ? node->right : node->left;

            traverse(ray, first.get(), hit);
            if (tr0 < hit.distance) {
                traverse(ray, second.get(), hit);
            }
        }
    }

};
#endif //PPGSO_BVH_H
