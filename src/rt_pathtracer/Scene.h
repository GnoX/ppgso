#ifndef PPGSO_SCENE_H
#define PPGSO_SCENE_H


#include <vector>
#include <bits/unique_ptr.h>
#include <map>
#include <glm/vec2.hpp>
#include "Object.h"
#include "Camera.h"

class Scene {
public:
    pathtracer::Camera camera;
    std::map<int, int> key;
    int key_mod;
    std::vector<std::unique_ptr<Object>> objects;
    struct {
        double x, y;
        bool left, right;
    } cursor, prev_cursor;
};


#endif //PPGSO_SCENE_H
