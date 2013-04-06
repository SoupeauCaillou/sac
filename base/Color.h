#pragma once
#if SAC_WINDOWS
#pragma warning(disable:4201) //disable warning about nameless unions
#endif

#include <string>
#include <iostream>

struct Color {
    public:
	union {
		struct {
			float rgba[4];
		};
		struct {
			float r, g, b, a;
		};
	};

 	static Color random();
	static void nameColor(const Color& c, const std::string& name);

	Color(float _r=1.0, float _g=1.0, float _b=1.0, float _a=1.0);
	Color(const std::string& name);

    Color operator*(float s) const;
    Color operator+(const Color& c) const;
    const Color& operator+=(const Color& c);
    bool operator==(const Color& c) const;

    bool operator!=(const Color& c) const;
    bool operator<(const Color& c) const;
    bool operator>(const Color& c) const;
};

inline std::ostream& operator<<(std::ostream& s, const Color& c) {
    unsigned int a;
    a = (((int)(255 * c.r)) & 0xff) << 24;
    a |= (((int)(255 * c.g)) & 0xff) << 16;
    a |= (((int)(255 * c.b)) & 0xff) << 8;
    a |= (((int)(255 * c.a)) & 0xff);
    s << std::hex << '{' << a << '}' << std::dec;
    return s;
}

inline std::istream& operator>>(std::istream& s, Color& c) {
    s >> c.r >> c.g >> c.b >> c.a;
    return s;
}

