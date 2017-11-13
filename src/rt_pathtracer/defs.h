#ifndef PPGSO_DEFS_H
#define PPGSO_DEFS_H

#include "stb_image.h"

#include <limits>
#include <cmath>

const double INF = std::numeric_limits<double>::infinity();
constexpr double EPS = std::numeric_limits<double>::epsilon();   // Numerical epsilon
const double DELTA = sqrt(EPS);                             // Delta to use

#endif //PPGSO_DEFS_H
