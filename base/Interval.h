#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/compatibility.hpp>
#include "base/Log.h"
#include "base/Color.h"

#if SAC_DEBUG
#define CHECK_ITV_RESULT LOGE_IF(r < t1 || r > t2, "Random value " << r << " outside [" << t1 << ',' << t2 << ']');
#else
#define CHECK_ITV_RESULT
#endif

template <typename T>
class Interval {
    public:
        Interval() {}
        Interval(T _t1, T _t2) : t1(_t1), t2(_t2) {}
        Interval(T _t) : t1(_t), t2(_t) {}

        T random() const;

        inline T lerp(float w) const {
            return lerp(t1, t2, w);
        }
        static inline T lerp(T _t1, T _t2, float w) {
            return _t1 + (_t2 - _t1) * w;
        }
        inline float position(T t) const {
            return (t - t1) / (t2 - t1);
        }

        T t1, t2;
};

template<>
inline glm::vec2 Interval<glm::vec2>::random() const {
    float w = glm::linearRand(0.0f, 1.0f);
    return lerp(w);
}

template<>
inline Color Interval<Color>::random() const {
    float w = glm::linearRand(0.0f, 1.0f);
    return lerp(w);
}

template<>
inline int Interval<int>::random() const {
    float w = glm::linearRand(0.0f, 1.0f);
    int r = (int) Interval::lerp(t1, t2 + 1, w);
    CHECK_ITV_RESULT
    return r;
}

template<>
inline bool Interval<bool>::random() const {
    float w = glm::linearRand(0.0f, 1.0f);
    bool r = w > 0.5;
    CHECK_ITV_RESULT
    return r;
}

template<class T>
inline T Interval<T>::random() const {
    float w = glm::linearRand(0.0f, 1.0f);
    T r = lerp(w);
    CHECK_ITV_RESULT
    return r;
}

#undef CHECK_ITV_RESULT