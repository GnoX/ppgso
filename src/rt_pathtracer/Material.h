#ifndef PPGSO_MATERIAL_H
#define PPGSO_MATERIAL_H

#include <glm/vec3.hpp>
#include <utility>
#include "HDRImage.h"

enum class MaterialType {
    DIFFUSE,
    SPECULAR,
    REFRACTIVE,
    REFLECTIVE_AND_REFRACTIVE,
    METALLIC_ROUGHNESS,
    MAPPED
};

struct Material {
    glm::vec3 emission, diffuse;
    MaterialType type = MaterialType::DIFFUSE;
    // TODO: also change this to non-hdr image
    std::shared_ptr<HDRImage3> albedo;
    std::shared_ptr<HDRImage3> normal_map;
    std::shared_ptr<HDRImage1> metalness;
    std::shared_ptr<HDRImage1> roughness;
    float transparency = 0;
    float ior = 1.4f;
    float fresnel = 0;
    glm::vec3 specular_color = {1.0f, 1.0f, 1.0f};

    Spectrum3 get_albedo(glm::uvec2 uv) const {
        if (albedo != nullptr) {
            return albedo->get_pixel(uv.x, uv.y);
        }
        return Spectrum3{diffuse.r, diffuse.g, diffuse.b};
    }

    Spectrum1 get_roughness(glm::uvec2 uv) const {
        if (roughness != nullptr) {
            return roughness->get_pixel(uv.x, uv.y);
        } else return Spectrum1();
    }

    Spectrum1 get_metalness(glm::uvec2 uv) const {
        if (metalness != nullptr) {
            return metalness->get_pixel(uv.x, uv.y);
        } else return Spectrum1();
    }

    static const Material new_mr_material(std::shared_ptr<HDRImage3> albedo,
                                          std::shared_ptr<HDRImage3> normal,
                                          std::shared_ptr<HDRImage1> metalness,
                                          std::shared_ptr<HDRImage1> roughness,
                                          float transparency = 0,
                                          float ior = 1.4f,
                                          glm::vec3 specular_color = {1.0f, 1.0f, 1.0f}) {
        return Material{{}, {}, MaterialType::METALLIC_ROUGHNESS, albedo,
            normal, metalness, roughness, transparency, ior, 0.0f,
            specular_color};
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
    static const Material ReflectiveAndRefractive() {
        return {{}, {}, MaterialType::REFLECTIVE_AND_REFRACTIVE};
    };
    static const Material Bricks() {
        return {{}, {}, MaterialType::METALLIC_ROUGHNESS,
                        std::make_shared<HDRImage3>("texture_bricks_diffuse.jpg"),
                nullptr,
                        std::make_shared<HDRImage1>("scuffs_metalness.jpg"),
                        std::make_shared<HDRImage1>("scuffs_roughness.jpg")};
    }

    static const Material MetalScuffs() {
        return {{}, {}, MaterialType::METALLIC_ROUGHNESS,
                        std::make_shared<HDRImage3>("scuffs_diffuse.jpg"),
                        nullptr,
//                        std::make_shared<HDRImage3>("scuffs_normal.jpg"),
                        std::make_shared<HDRImage1>("scuffs_metalness.jpg"),
                        std::make_shared<HDRImage1>("scuffs_roughness.jpg")};
    }

    static const Material new_mr_material(float metalness_value, float roughness_value, glm::vec3 color) {
        auto metalness = std::make_shared<HDRImage1>(1, 1);
        auto roughness = std::make_shared<HDRImage1>(1, 1);
        auto diffuse = std::make_shared<HDRImage3>(1, 1);
        metalness->set_pixel({metalness_value}, 0, 0);
        roughness->set_pixel({roughness_value}, 0, 0);
        diffuse->set_pixel(Spectrum3::from_vec(color), 0, 0);
        return new_mr_material(diffuse, nullptr, metalness, roughness);

    }

};
#endif //PPGSO_MATERIAL_H
