/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once
#if SAC_WINDOWS
//disable warning about nameless unions
//#pragma warning(disable:4201)
#endif

#include <string>
#include <iostream>
#include <base/SacDefs.h>
#include <glm/glm.hpp>

struct color_comp {
        uint8_t value;

        #define op(x) (uint8_t)glm::max(0, glm::min(255, int(x)))
        color_comp operator-(color_comp o) const { color_comp c; c.value = op(value - o.value); return c;}
        color_comp operator+(color_comp o) const { color_comp c; c.value = op(value + o.value); return c;}
        color_comp& operator+=(color_comp o) { value = op(value + o.value); return *this;}
        color_comp operator*(float s) const { color_comp c; c.value = op(value * s); return c;}
        color_comp& operator*=(float s) { value = op(value * s); return *this;}

        //color_comp& operator=(color_comp& o) { value = o.value; return *this;}
        color_comp& operator=(int v) { value = v; return *this;}
        color_comp& operator=(float v) { value = op(255*v); return *this; }

        explicit operator float() const {
            constexpr float ratio = 1.0f / 255.0f;
            return ratio * value;
        }
};

struct Color {
    public:
    PRAGMA_WARNING(warning(disable: 4201))
    union {
        struct {
            color_comp rgba[4];
        };
        struct {
            color_comp r, g, b, a;
        };
    };

    static Color random();
    static void nameColor(const Color& c, const std::string& name);

    Color(uint8_t _r=255, uint8_t _g=255, uint8_t _b=255, uint8_t _a=255);
    Color(int _r, int _g, int _b, int _a = 255);
    Color(color_comp r, color_comp g, color_comp b, color_comp a);
    explicit Color(float _r, float _g, float _b, float _a=1.0);
    Color(float* rgba, uint32_t mask);
    Color(const std::string& name);

    Color operator*(float s) const;
    Color operator+(const Color& c) const;
    Color operator-(const Color& c) const;
    const Color& operator+=(const Color& c);
    bool operator==(const Color& c) const;

    bool operator!=(const Color& c) const;
    bool operator<(const Color& c) const;
    bool operator>(const Color& c) const;

    uint32_t asInt() const;

    bool isGrey(float epsilon = 0.001f) const;

    void reducePrecision(float maxPrecision);
};

inline std::ostream& operator<<(std::ostream& s, const Color& c) {
    return s << "color = [r:" << c.r.value
        << ", g:" << c.g.value
        << ", b:" << c.b.value
        << ", a:" << c.a.value << ']';
}

inline std::istream& operator>>(std::istream& s, Color& c) {
    s >> c.r.value >> c.g.value >> c.b.value >> c.a.value;
    return s;
}

