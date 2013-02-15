#include "Color.h"

#include "MathUtil.h"
#include <cstring>
#include <glog/logging.h>
#include <map>

static std::map<std::string, Color> name2Color;

Color Color::random() {
	return (Color( MathUtil::RandomFloat(), MathUtil::RandomFloat(), MathUtil::RandomFloat()));
}

void Color::nameColor(const Color& c, const std::string& name) {
    VLOG(1) << "Add color " << name << " = " << c;
	CHECK(name2Color.insert(std::make_pair(name, c)).second) << "Warning, color with name " << name << " already existed.";
}

Color::Color(float _r, float _g, float _b, float _a):
	r(_r), g(_g), b(_b), a(_a) {
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

bool Color::operator!=(const Color& c) const {
	return memcmp(rgba, c.rgba, sizeof(rgba)) != 0;
}
bool Color::operator<(const Color& c) const {
	return memcmp(rgba, c.rgba, sizeof(rgba)) < 0;
}

bool Color::operator==(const Color& c) const {
	return (memcmp(rgba, c.rgba, sizeof(rgba))==0);
}
