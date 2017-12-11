#ifndef PPGSO_SCENE_H
#define PPGSO_SCENE_H


#include <vector>
#include <bits/unique_ptr.h>
#include <map>
#include <glm/vec2.hpp>
#include "Object.h"
#include "Camera.h"
#include "TracableObject.h"

class Scene {
public:
    pathtracer::Camera camera;
    std::map<int, int> key;
    std::map<int, int> last_key;
    int key_mod;

    std::vector<std::unique_ptr<TracableObject>> objects;
    struct {
        double x, y;
        bool left, right;
    } cursor, prev_cursor;

    template<typename T>
    void add(T object) {
        objects.emplace_back(std::make_unique<T>(std::move(object)));
    }
};


#endif //PPGSO_SCENE_H
