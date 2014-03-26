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



#include "Color.h"

#include "util/Random.h"
#include <cstring>
#include "Log.h"
#include <map>

static std::map<std::string, Color> name2Color;

Color Color::random() {
    return Color(Random::Int(0, 255), Random::Int(0, 255), Random::Int(0, 255), Random::Int(0, 255));
}

void Color::nameColor(const Color& c, const std::string& name) {
    LOGV(1, "Add color " << name << " = " << c);
    name2Color.insert(std::make_pair(name, c));
}

Color::Color(color_comp _r, color_comp _g, color_comp _b, color_comp _a) :
    r(_r), g(_g), b(_b), a(_a) {}

Color::Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) {
    r.value = _r;
    g.value = _g;
    b.value = _b;
    a.value = _a;
}

Color::Color(int _r, int _g, int _b, int _a) {
    r.value = _r;
    g.value = _g;
    b.value = _b;
    a.value = _a;
}

Color::Color(float _r, float _g, float _b, float _a) {
    r = _r;
    g = _g;
    b = _b;
    a = _a;
}

Color::Color(float* pRgba, uint32_t) {
    LOGV(1, "Color mask ignored");
    r = pRgba[0];
    g = pRgba[1];
    b = pRgba[2];
    a = pRgba[3];
}

Color::Color(const std::string& name) {
    std::map<std::string, Color>::iterator it = name2Color.find(name);
    if (it != name2Color.end()) {
        *this = it->second;
    } else {
        r = g = b = a = 1.0f;
    }
}

Color Color::operator*(float s) const {
    return Color(r*s, g*s, b*s, a*s);
}
Color Color::operator+(const Color& c) const {
    return Color(r+c.r, g+c.g, b+c.b, a+c.a);
}

Color Color::operator-(const Color& c) const {
    return Color(r-c.r, g-c.g, b-c.b, a-c.a);
}

const Color& Color::operator+=(const Color& c) {
    r += c.r;
    g += c.g;
    b += c.b;
    a += c.a;
    return *this;
}

bool Color::operator!=(const Color& c) const {
    return memcmp(rgba, c.rgba, sizeof(rgba)) != 0;
}
bool Color::operator<(const Color& c) const {
    return memcmp(rgba, c.rgba, sizeof(rgba)) < 0;
}
bool Color::operator>(const Color& c) const {
    return memcmp(rgba, c.rgba, sizeof(rgba)) > 0;
}

bool Color::operator==(const Color& c) const {
    return (memcmp(rgba, c.rgba, sizeof(rgba))==0);
}

uint32_t Color::asInt() const {
    uint32_t result =
        ((((uint32_t)(r.value))& 0xFF) << 24)
        | ((((uint32_t)(g.value))& 0xFF) << 16)
        | ((((uint32_t)(b.value))& 0xFF) << 8)
        | (((uint32_t)(a.value))& 0xFF);
    return result;
}

bool Color::isGrey(float epsilon) const {
    return (glm::abs(r.value - g.value) < epsilon * 255) &&
        (glm::abs(r.value - b.value) < epsilon * 255) &&
        (glm::abs(g.value - g.value) < epsilon * 255);
}

void Color::reducePrecision(float maxPrecision) {
    LOGT("TODO");
    #if 0
    LOGF_IF(maxPrecision <= 0 || maxPrecision >= 1, "Invalid maxPrecision value:" << maxPrecision);
    const float inc = 1.0f / maxPrecision;
    for (int i=0; i<4; i++) {
        rgba[i] = maxPrecision * (int) (glm::round(rgba[i] * inc));
    }
    #endif
}
