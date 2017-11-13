#ifndef PPGSO_SCENE_H
#define PPGSO_SCENE_H


#include <vector>
#include <bits/unique_ptr.h>
#include "Object.h"

class Scene {

public:
    std::vector<std::unique_ptr<Object>> objects;
};


#endif //PPGSO_SCENE_H
