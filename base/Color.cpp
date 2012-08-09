#include "Color.h"

#include "MathUtil.h"
#include <cstring>

Color Color::random() {
	return (Color( MathUtil::RandomFloat(), MathUtil::RandomFloat(), MathUtil::RandomFloat()));
}

Color::Color(float _r=1.0, float _g=1.0, float _b=1.0, float _a=1.0):
	r(_r), g(_g), b(_b), a(_a) {
}

Color Color::operator*(float s) const {
	return Color(r*s, g*s, b*s, a*s);
}
Color Color::operator+(const Color& c) const {
	return Color(r+c.r, g+c.g, b+c.b, a+c.a);
}

bool Color::operator!=(const Color& c) const {
	return memcmp(rgba, c.rgba, sizeof(rgba)) != 0;
}
bool Color::operator<(const Color& c) const {
	return memcmp(rgba, c.rgba, sizeof(rgba)) < 0;
}
