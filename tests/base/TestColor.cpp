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



#include <UnitTest++.h>
#include "base/Color.h"

TEST(ColorIsGrey)
{
        Color c(0.2, 0.2, 0.2);
        CHECK(c.isGrey());
}

TEST(ColorReducePrecision)
{
        Color c(0.243356, 0.123545, 0.974213);
        c.reducePrecision(0.1);
        CHECK_CLOSE(0.2, c.r, 0.0001);
        CHECK_CLOSE(0.1, c.g, 0.0001);
        CHECK_CLOSE(1.0, c.b, 0.0001);
}