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

struct Color {
    public:
        PRAGMA_WARNING(warning(disable: 4201))
        union {
                struct {
                        float rgba[4];
                };
                struct {
                        float r, g, b, a;
                };
        };

        static Color random(float _a=1.f);
        static void nameColor(const Color& c, const std::string& name);

        Color(float _r=1.0, float _g=1.0, float _b=1.0, float _a=1.0);
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

    const Color & reducePrecision(float maxPrecision);
};

inline std::ostream& operator<<(std::ostream& s, const Color& c) {
    return s << "color = [r:" << (int)(c.r * 255)
        << ", g:" << (int)(c.g * 255)
        << ", b:" << (int)(c.b * 255)
        << ", a:" << (int)(c.a * 255)<< ']';
}

inline std::istream& operator>>(std::istream& s, Color& c) {
    s >> c.r >> c.g >> c.b >> c.a;
    return s;
}

