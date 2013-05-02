#pragma once

template <typename T>
class Frequency {
    public:
        Frequency(T inV) : value(inV), accum(0.) {}

    T value;
    float accum;
};
