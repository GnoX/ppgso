#ifndef PPGSO_TEXTUREMAP_H
#define PPGSO_TEXTUREMAP_H

#include <algorithm>
#include <vector>
#include <glm/glm.hpp>
#include <stdexcept>
#include "defs.h"

struct Image {

    struct Color {
        unsigned char r, g, b;
    };

    unsigned w;
    unsigned h;
    std::vector<Color> data;

    Image() : w(0), h(0) {}
    Image(unsigned w, unsigned h) : w(w), h(h) { data.resize(w * h); }

    explicit Image(const std::string &filename) {
        int w, h, n;
        auto image = reinterpret_cast<Color*>(stbi_load(filename.c_str(), &w, &h, &n, 0));
        if (n != 3) throw std::runtime_error("Cannot handle specified amount of channels");

        if (image != nullptr) {
            data = std::vector<Color>(image, image + w * h);
            this->w = (unsigned) w;
            this->h = (unsigned) h;
        }
    }

    void resize(unsigned w, unsigned h) {
        this->w = w;
        this->h = h;
        data.resize(w * h);
        clear();
    }

    void set_pixel(const Color& c, unsigned x, unsigned y) {
        data[x + y * w].r = c.r;
        data[x + y * w].g = c.g;
        data[x + y * w].b = c.b;
    }

    Color get_pixel(unsigned x, unsigned y) {
        return data[x + y * w];
    }

    void clear() { std::fill(data.begin(), data.end(), Color{0, 0, 0}); }
};


#endif //PPGSO_TEXTUREMAP_H
