#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

template <typename T>
class Interval {
    public:
        Interval() {}
        Interval(T _t1, T _t2) : t1(_t1), t2(_t2) {}
        Interval(T _t) : t1(_t), t2(_t) {}

        T random() const {
            float w = glm::linearRand(0.0f, 1.0f);
            return lerp(w);
        }

        inline T lerp(float w) const {
            return t1 * (1-w) + t2 * w;
        }
        inline float position(T t) const {
            return (t - t1) / (t2 - t1);
        }

        T t1, t2;
};
