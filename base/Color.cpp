#include "Color.h"

#include <glm/gtc/random.hpp>
#include <cstring>
#include "Log.h"
#include <map>

static std::map<std::string, Color> name2Color;

Color Color::random() {
	return (Color(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f)));
}

void Color::nameColor(const Color& c, const std::string& name) {
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

Color::Color(const std::string& name) {
	std::map<std::string, Color>::iterator it = name2Color.find(name);
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

bool Color::isGrey(float epsilon) const {
	return (glm::abs(r - g) < epsilon) &&
		(glm::abs(r -b) < epsilon) &&
		(glm::abs(g - g) < epsilon);
}