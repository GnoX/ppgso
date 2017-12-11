#ifndef PPGSO_HDRIMAGE_H
#define PPGSO_HDRIMAGE_H

#include <cmath>
#include <vector>
#include <image.h>
#include "defs.h"
#include "Spectrum.h"

template <unsigned int Channels>
struct HDRImage {
    unsigned w;
    unsigned h;
    std::vector<Spectrum<Channels>> data;

    HDRImage() : w(0), h(0) { }

    HDRImage(unsigned w, unsigned h) : w(w), h(h) { data.resize(w * h); }

    explicit HDRImage(const std::string& filename) {
        int w, h, n;
        Spectrum<Channels>* image = reinterpret_cast<Spectrum<Channels>*>(stbi_loadf(filename.c_str(), &w, &h, &n, 0));
        if (n != Channels) throw std::runtime_error("Cannot handle specified amount of channels");

        if (image != nullptr) {
            data = std::vector<Spectrum<Channels>>(image, image + w * h);
            this->w = (unsigned) w;
            this->h = (unsigned) h;
        }
    }

    inline void resize(unsigned w, unsigned h) {
        this->w = w;
        this->h = h;
        data.resize(w * h);
        clear();
    }

    inline Spectrum<Channels> get_pixel(unsigned x, unsigned y) const {
        return data[x + y * w];
    }

    inline void set_pixel(const Spectrum<Channels>& s, unsigned x, unsigned y) {
        data[x + y * w] = s;
    }

    void tonemap(ppgso::Image& target,
                 float gamma, float level, float key, float white) {
        float avg = 0;
        for (unsigned i = 0; i < w * h; ++i) {
            avg += std::log(0.0000001f + data[i].illumination());
        }
        avg = std::exp(avg / (w * h));


        float one_over_gamma = 1.0f / gamma;
        auto exposure = static_cast<float>(sqrt(pow(2, level)));
        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                Spectrum<Channels> s = data[x + y * w];
                float l = s.illumination();
                s *= key / avg;
                s *= ((l + 1) / (white * white)) / (l + 1);
                float r = std::pow(s.c[0] * exposure, one_over_gamma);
                float g = std::pow(s.c[1] * exposure, one_over_gamma);
                float b = std::pow(s.c[2] * exposure, one_over_gamma);
                target.setPixel(x, y, r, g, b);
            }
        }
    }

    void toColorImage(ppgso::Image& target, unsigned x0, unsigned y0, unsigned x1, unsigned y1) {
        float gamma = 2.2f;
        float level = 1.0f;
        float one_over_gamma = 1.0f / gamma;
        auto exposure = static_cast<float>(sqrt(pow(2, level)));
        for (unsigned y = y0; y < y1; ++y) {
            for (unsigned x = x0; x < x1; ++x) {
                const Spectrum<Channels>& s = data[x + y * w];
                float r = std::pow(s.c[0] * exposure, one_over_gamma);
                float g = std::pow(s.c[1] * exposure, one_over_gamma);
                float b = std::pow(s.c[2] * exposure, one_over_gamma);
                target.setPixel(x, y, r, g, b);
            }
        }
    }

    inline bool empty() const { return w == 0 && h == 0; };

    void clear() { std::fill(data.begin(), data.end(), Spectrum<Channels>{0}); }

};
typedef HDRImage<3> HDRImage3;
typedef HDRImage<1> HDRImage1;

#endif //PPGSO_HDRIMAGE_H
