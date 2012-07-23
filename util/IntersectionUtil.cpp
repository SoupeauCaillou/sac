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

#include "IntersectionUtil.h"
#include "../base/MathUtil.h"

bool IntersectionUtil::pointRectangle(const Vector2& point, const Vector2& rectPos, const Vector2& rectSize) {
	return (MathUtil::Abs(rectPos.X - point.X) < rectSize.X * 0.5 &&
		MathUtil::Abs(rectPos.Y - point.Y) < rectSize.Y * 0.5);
}

// from http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
bool IntersectionUtil::lineLine(const Vector2& pA, const Vector2& pB, const Vector2& qA, const Vector2& qB, Vector2* intersectionPoint) {
	float denom = ((qB.Y - qA.Y)*(pB.X - pA.X)) -
                      ((qB.X - qA.X)*(pB.Y - pA.Y));

    float nume_a = ((qB.X - qA.X)*(pA.Y - qA.Y)) -
                   ((qB.Y - qA.Y)*(pA.X - qA.X));

    float nume_b = ((pB.X - pA.X)*(pA.Y - qA.Y)) -
                   ((pB.Y - pA.Y)*(pA.X - qA.X));

    if(denom == 0.0f)
    {
        if(nume_a == 0.0f && nume_b == 0.0f)
        {
            return false;
        }
        return false;
    }

    float ua = nume_a / denom;
    float ub = nume_b / denom;

    if(ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f)
    {
    	if (intersectionPoint) {
            // Get the intersection point.
            intersectionPoint->X = pA.X + ua*(pB.X - pA.X);
            intersectionPoint->Y = pA.Y + ua*(pB.Y - pA.Y);
        }

        return true;
    }

    return false;
}
