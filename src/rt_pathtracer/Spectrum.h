#ifndef PPGSO_SPECTRUM_H
#define PPGSO_SPECTRUM_H

#include <glm/vec3.hpp>
#include <glm/glm.hpp>

struct Spectrum {
    float r, g, b;

    void clamp(float min, float max) {
        r = glm::clamp(r, min, max);
        g = glm::clamp(g, min, max);
        b = glm::clamp(b, min, max);
    }

    inline float illumination() const {
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
    }

    inline Spectrum operator*(float s) const {
        return Spectrum{r * s, g * s, b * s};
    }

    inline Spectrum operator*(Spectrum other) {
        return Spectrum{r * other.r, g * other.g, b * other.b};
    }

    inline Spectrum &operator*=(float s) {
        r *= s;
        g *= s;
        b *= s;
        return *this;
    }

    inline Spectrum &operator+=(Spectrum s) {
        r += s.r;
        g += s.g;
        b += s.b;
        return *this;
    }

    inline Spectrum operator+(glm::vec3 o) {
        return Spectrum{o.r + r, o.g + g, o.b + b};
    }

    inline Spectrum operator*(glm::vec3 o) {
        return Spectrum{o.r * r, o.g * g, o.b * b};
    }

    inline Spectrum operator/(float d) {
        return Spectrum{r / d, g / d, b / d};
    }

    inline Spectrum &operator/=(float d) {
        r /= d;
        g /= d;
        b /= d;
        return *this;
    }
};
#endif //PPGSO_SPECTRUM_H
