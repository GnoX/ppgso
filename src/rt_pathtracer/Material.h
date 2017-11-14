#ifndef PPGSO_MATERIAL_H
#define PPGSO_MATERIAL_H

#include <glm/vec3.hpp>
#include "HDRImage.h"

enum class MaterialType {
    DIFFUSE,
    SPECULAR,
    REFRACTIVE
};

/*!
 * Material coefficients for diffuse and emission
 */
struct Material {
    glm::vec3 emission, diffuse;
    MaterialType type = MaterialType::DIFFUSE;
    HDRImage diffuse_map;
    HDRImage normal_map;

    Spectrum get_diffuse(glm::uvec2 uv) const {
        if (!diffuse_map.empty()) {
            return diffuse_map.get_pixel(uv.x, uv.y);
        }
        return Spectrum{diffuse.r, diffuse.g, diffuse.b};
    }

    static const Material Light() {
        return {{1, 1, 1}};
    };
    static const Material Red() {
        return {{}, {1, 0, 0}};
    };
    static const Material Green() {
        return {{}, {0, 1, 0}};
    };
    static const Material Blue() {
        return {{}, {0, 0, 1}};
    };
    static const Material Yellow() {
        return {{}, {1, 1, 0}};
    };
    static const Material Magenta() {
        return {{}, {1, 0, 1}};
    };
    static const Material Cyan() {
        return {{}, {0, 1, 1}};
    };
    static const Material White() {
        return {{}, {1, 1, 1}};
    };
    static const Material Gray() {
        return {{}, {.5, .5, .5}};
    };
    static const Material Mirror() {
        return {{}, {}, MaterialType ::SPECULAR};
    };
    static const Material Glass() {
        return {{}, {}, MaterialType::REFRACTIVE};
    };
    static const Material Bricks() {
        return {{}, {}, MaterialType::DIFFUSE,
                HDRImage("texture_bricks_diffuse.jpg"),
                HDRImage("texture_bricks_normal.jpg")};
    }
};
#endif //PPGSO_MATERIAL_H
