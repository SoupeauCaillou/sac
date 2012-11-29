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
#include "../systems/TransformationSystem.h"
#include <cmath>

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

bool IntersectionUtil::rectangleRectangle(const Vector2& rectAPos, const Vector2& rectASize, float rectARot, 
            const Vector2& rectBPos, const Vector2& rectBSize, float rectBRot) {
    // quick out
    if (Vector2::DistanceSquared(rectAPos, rectBPos) > pow(MathUtil::Max(rectASize.X, rectASize.Y) + MathUtil::Max(rectBSize.X, rectBSize.Y), 2)) {
        return false;
    }
    
    #define SIGN(x) ((x) > 0 ? 1:-1)
    const float coeff[] = {
        -0.5, 0.5,            
        0.5, 0.5,
        0.5, -0.5,
        -0.5, -0.5
    };
    
    Vector2 aPoints[4];
    Vector2 bPoints[4];
    
    for (int i=0; i<4; i++) {
        aPoints[i] = rectAPos + Vector2::Rotate(Vector2(rectASize.X * coeff[2*i], rectASize.Y * coeff[2*i+1]), rectARot);
        bPoints[i] = rectBPos + Vector2::Rotate(Vector2(rectBSize.X * coeff[2*i], rectBSize.Y * coeff[2*i+1]), rectBRot);
    }
    
    // check A edges againts B points
    for (int i=0; i<4; i++) {
        Vector2 edge(aPoints[(i+1)%4] - aPoints[i]);
        float tmp = edge.X;
        edge.X = -edge.Y;
        edge.Y = tmp;
        
        bool success = true;
        int j, side = SIGN(Vector2::Dot(edge, bPoints[0] - aPoints[i]));
        for (j=1; success && j<4; j++) {
            int d = SIGN(Vector2::Dot(edge, bPoints[j] - aPoints[i]));
            success = (d == side);
        }
        // all points from B are on the same side
        // if at least one point of A is on the other side we're good
        if (success && j == 4) {
            int d1 = SIGN(Vector2::Dot(edge, aPoints[(i+2) % 4] - aPoints[i]));
            if (d1 != side)
                return false;
            int d2 = SIGN(Vector2::Dot(edge, aPoints[(i+3) % 4] - aPoints[i]));
            if (d2 != side)
                return false;
        }
    }
    
    // check B edges againts A points
    for (int i=0; i<4; i++) {
        Vector2 edge(bPoints[(i+1)%4] - bPoints[i]);
        float tmp = edge.X;
        edge.X = -edge.Y;
        edge.Y =tmp;
        
        bool success = true;
        int j, side = SIGN(Vector2::Dot(edge, aPoints[0] - bPoints[i]));
        for (j=1; success && j<4; j++) {
            int d = SIGN(Vector2::Dot(edge, aPoints[j] - bPoints[i]));
            success = (d == side);
        }
        // all points from A are on the same side
        // if at least one point of B is on the other side we're good
        if (success && j == 4) {
            int d1 = SIGN(Vector2::Dot(edge, bPoints[(i+2) % 4] - bPoints[i]));
            if (d1 != side)
                return false;
            int d2 = SIGN(Vector2::Dot(edge, bPoints[(i+3) % 4] - bPoints[i]));
            if (d2 != side)
                return false;
        }
    }
    return true;
}

bool IntersectionUtil::rectangleRectangle(const TransformationComponent* tc1, const TransformationComponent* tc2) {
    return rectangleRectangle(
        tc1->worldPosition, tc1->size, tc1->worldRotation,
        tc2->worldPosition, tc2->size, tc2->worldRotation);
}