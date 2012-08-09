#pragma once

struct Color {
	union {
		struct {
			float rgba[4];
		};
		struct {
			float r, g, b, a;
		};
	};

 	static Color random() {
 		return (Color( MathUtil::RandomFloat(), MathUtil::RandomFloat(), MathUtil::RandomFloat()));
 	}

	Color(float _r=1.0, float _g=1.0, float _b=1.0, float _a=1.0):
		r(_r), g(_g), b(_b), a(_a) {}

     Color operator*(float s) const {
        return Color(r*s, g*s, b*s, a*s);
     }
     Color operator+(const Color& c) const {
        return Color(r+c.r, g+c.g, b+c.b, a+c.a);
     }

     bool operator!=(const Color& c) const {
        return memcmp(rgba, c.rgba, sizeof(rgba)) != 0;
     }
     bool operator<(const Color& c) const {
        return memcmp(rgba, c.rgba, sizeof(rgba)) < 0;
     }
};

