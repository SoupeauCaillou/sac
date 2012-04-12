#pragma once

#include "MathUtil.h"

template <typename T>
class Interval {
    public:
        Interval() {}
        Interval(T _t1, T _t2) : t1(_t1), t2(_t2) {}

        T random() const {
            float w = MathUtil::RandomFloat();
            return lerp(w);
        }

        inline T lerp(float w) const {
            return t1 * (1-w) + t2 * w;
        }

        T t1, t2;
};
