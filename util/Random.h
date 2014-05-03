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
    static void Init (unsigned int seed = 0);


    // return value is between [min; max]
    static float Float (float min=0.0f, float max=1.0f);

    static void N_Floats(int n, float* out, float min=0.0f, float max=1.0f);

    // return value is between [min; max]
	static int Int (int min=0, int max=1);

    static void N_Ints(int n, int* out, int min=0.0f, int max=1.0f);
};
