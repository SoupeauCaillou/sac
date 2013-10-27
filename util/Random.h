/*
    This file is part of Heriswap.

    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

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

class Random {
	public:
	static float Float (float Min=0.0f, float Max=1.0f);
	static int Int (int Min=0, int Max=1);
};
