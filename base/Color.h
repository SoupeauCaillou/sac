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

 	static Color random();

	Color(float _r=1.0, float _g=1.0, float _b=1.0, float _a=1.0);

     Color operator*(float s) const;
     Color operator+(const Color& c) const;

     bool operator!=(const Color& c) const;
     bool operator<(const Color& c) const;
};

