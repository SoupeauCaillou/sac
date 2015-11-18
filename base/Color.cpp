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

#include <glm/gtc/random.hpp>
#include <cstring>
#include "Log.h"
#include <map>

static std::map<hash_t, Color> name2Color;

Color Color::palette(float t, float alpha) {
    glm::vec3 a(0.5f, 0.5f, 0.5f);
    glm::vec3 b(0.5f, 0.5f, 0.5f);
    glm::vec3 c(1.0f, 1.0f, 1.0f);
    glm::vec3 d(0.0f, 0.1f, 0.2f);

    Color result;
    for (int i=0; i<3; i++) {
        result.rgba[i] =
            a[i] + b[i] * glm::cos(2 * 3.14157 * (c[i] * t + d[i]));
    }
    result.a = alpha;
    return result;
}

Color Color::random(float alpha) {
        return Color(
            glm::linearRand(0.0f, 1.0f)
            , glm::linearRand(0.0f, 1.0f)
            , glm::linearRand(0.0f, 1.0f)
            , alpha
        );
}

void Color::nameColor(const Color& c, hash_t name) {
    LOGV(1, "Add color " << name << " = " << c);
        name2Color.insert(std::make_pair(name, c));
}

Color::Color(float _r, float _g, float _b, float _a):
        r(_r), g(_g), b(_b), a(_a) {
}

Color::Color(float* pRgba, uint32_t) {
    LOGV(1, "Color mask ignored");
    memcpy(rgba, pRgba, 4 * sizeof(float));
}

Color::Color(hash_t name) {
    std::map<hash_t, Color>::iterator it = name2Color.find(name);
    if (it != name2Color.end()) {
            *this = it->second;
    } else {
            r = g = b = a = 1.0;
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
        ((((uint32_t)(r * 255))& 0xFF) << 24)
        | ((((uint32_t)(g * 255))& 0xFF) << 16)
        | ((((uint32_t)(b * 255))& 0xFF) << 8)
        | (((uint32_t)(a * 255))& 0xFF);
    return result;
}

bool Color::isGrey(float epsilon) const {
        return (glm::abs(r - g) < epsilon) &&
                (glm::abs(r -b) < epsilon) &&
                (glm::abs(g - g) < epsilon);
}

const Color & Color::reducePrecision(float maxPrecision) {
    LOGF_IF(maxPrecision <= 0 || maxPrecision >= 1, "Invalid maxPrecision value:" << maxPrecision);
    const float inc = 1.0f / maxPrecision;
    for (int i=0; i<4; i++) {
        rgba[i] = maxPrecision * (int) (glm::round(rgba[i] * inc));
    }
    return *this;
}
