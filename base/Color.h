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

    bool isGrey(float epsilon = 0.001f) const;
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

