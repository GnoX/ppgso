#ifndef PPGSO_SPECTRUM_H
#define PPGSO_SPECTRUM_H

#include <glm/vec3.hpp>
#include <glm/glm.hpp>

template <unsigned int Channels>
struct Spectrum {
    float c[Channels];

    void clamp(float min, float max) {
        for (int i = 0; i < Channels; i++) { c[i] = glm::clamp(c[i], min, max); }
    }

    glm::vec3 to_vec3() {
        assert(Channels == 3);
        return glm::vec3(c[0], c[1], c[2]);
    }

    static Spectrum<Channels> from_vec(glm::vec3 v) {
        Spectrum<Channels> s;
        for (int i = 0; i < Channels; i++) {
            s.c[i] = v[i];
        }
        return s;
    }

    inline float illumination() const {
        assert(Channels == 3);
        return 0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2];
    }

    inline Spectrum<Channels> operator*(float s) const {
        auto ret = Spectrum<Channels>();
        for (int i = 0; i < Channels; i++) { ret.c[i] = c[i] * s; }
        return ret;
    }

    inline Spectrum<Channels> operator*(Spectrum<Channels> other) {
        auto ret = Spectrum<Channels>();
        for (int i = 0; i < Channels; i++) { ret.c[i] = c[i] * other.c[i]; }
        return ret;
    }

    inline Spectrum<Channels> &operator*=(float s) {
        for (int i = 0; i < Channels; i++) { c[i] *= s; }
        return *this;
    }

    inline Spectrum<Channels> &operator+=(Spectrum s) {
        for (int i = 0; i < Channels; i++) { c[i] += s.c[i]; }
        return *this;
    }

    inline Spectrum<Channels> operator+(Spectrum s) {
        auto ret = Spectrum<Channels>();
        for (int i = 0; i < Channels; i++) { ret.c[i] = c[i] + s.c[i]; }
        return ret;
    }

    inline Spectrum<Channels> operator+(glm::vec3 o) {
        assert(Channels == 3);
        return Spectrum<3>{o.r + c[0], o.g + c[1], o.b + c[2]};
    }

    inline Spectrum<Channels> operator*(glm::vec3 o) {
        assert(Channels == 3);
        return Spectrum<3>{o.r * c[0], o.g * c[1], o.b * c[2]};
    }

    inline Spectrum<Channels> operator/(float d) {
        auto ret = Spectrum<Channels>();
        for (int i = 0; i < Channels; i++) { ret.c[i] = c[i] / d; }
        return ret;
    }

    inline float& operator[](const int index) {
        assert(index < Channels);
        assert(index >= 0);
        return c[index];
    }

    inline Spectrum<Channels> &operator/=(float d) {
        for (int i = 0; i < Channels; i++) { c[i] /= d; }
        return *this;
    }
};

typedef Spectrum<3> Spectrum3;
typedef Spectrum<1> Spectrum1;
#endif //PPGSO_SPECTRUM_H
