/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "MathUtil.h"

template <typename T>
class Interval {
    public:
        Interval() {}
        Interval(T _t1, T _t2) : t1(_t1), t2(_t2) {}
        Interval(T _t) : t1(_t), t2(_t) {}

        T random() const {
            float w = MathUtil::RandomFloat();
            return lerp(w);
        }

        inline T lerp(float w) const {
            return t1 * (1-w) + t2 * w;
        }

        T t1, t2;
};
