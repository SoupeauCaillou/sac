#include "IntersectionUtil.h"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "../systems/TransformationSystem.h"
#include <cmath>

const float eps = 0.00001;

bool IntersectionUtil::pointLine(const glm::vec2& point, const glm::vec2& qA, const glm::vec2& qB) {
    const float norm2 = glm::length2(qB - qA);

    // if qA = qB, this is not a segment!
    if (norm2 < eps) {
        return (point == qA);
    } else {

        const float t = glm::dot (point - qA, qB - qA) / norm2;

        if (t >= -eps && t <= 1.f + eps) {
            return (glm::length2(point - (qA + t * (qB - qA))) < eps);
        }
    }
    return false;
}

bool IntersectionUtil::pointRectangle(const glm::vec2& point, const TransformationComponent* tc2 ) {
    return pointRectangle(point, tc2->position, tc2->size, tc2->rotation);
}

bool IntersectionUtil::pointRectangle(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectSize, float rectRotation) {
    glm::vec2 p(glm::rotate(point - rectPos, -rectRotation));

	return (glm::abs(p.x) < rectSize.x * 0.5 &&
		glm::abs(p.y) < rectSize.y * 0.5);
}

// from http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
bool IntersectionUtil::lineLine(const glm::vec2& pA, const glm::vec2& pB,
            const glm::vec2& qA, const glm::vec2& qB, glm::vec2* intersectionPoint, bool pIsStraigth, bool qIsStraigth) {
	float denom = ((qB.y - qA.y)*(pB.x - pA.x)) -
                      ((qB.x - qA.x)*(pB.y - pA.y));

    float nume_a = ((qB.x - qA.x)*(pA.y - qA.y)) -
                   ((qB.y - qA.y)*(pA.x - qA.x));

    float nume_b = ((pB.x - pA.x)*(pA.y - qA.y)) -
                   ((pB.y - pA.y)*(pA.x - qA.x));

    //line are parallels - need to check if they are coincidents or not
    if(glm::abs(denom) < eps)
    {
        // they are coincidents if qA is in [pA, pB] or if qB is in [pA, pB]
        const float pDeltaX = pB.x - pA.x;
        const float pDeltaY = pB.y - pA.y;

        //if the line is not totally VERTICALE, use the normal approach
        if (glm::abs(pDeltaX) > eps) {
            const float ua = (qA.x - pA.x) / pDeltaX;
            const float ub = (qB.x - pA.x) / pDeltaX;

            //is qA in segment?
            if ((pIsStraigth || (ua >= -eps && ua <= 1.f + eps)) && qA.y == pA.y + ua * pDeltaY) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = pA + (pB - pA) * ua;
                }
                return true;
            // or qB?
            } else if ((pIsStraigth || (ub >= -eps && ub <= 1.f + eps)) && qB.y == pA.y + ub * pDeltaY) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = pA + (pB - pA) * ub;
                }
                return true;
            }
        //vertical line, special case. Just need to check if they are on the same X
        } else if (glm::abs(qA.x - pA.x) < eps) {
            const float ua = (qA.y - pA.y) / pDeltaY;
            const float ub = (qB.y - pA.y) / pDeltaY;

            if (pIsStraigth || (ua >= -eps && ua <= 1.f + eps)) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = pA + (pB - pA) * ua;
                }
                return true;
            } else if (pIsStraigth || (ub >= -eps && ub <= 1.f + eps)) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = pA + (pB - pA) * ub;
                }
                return true;
            }
        }

        return false;
    }

    float ua = nume_a / denom;
    float ub = nume_b / denom;

    if (!pIsStraigth && (ua < -eps || ua > 1.f + eps)) {
        return false;
    }
    if (!qIsStraigth && (ub < -eps || ub > 1.f + eps)) {
        return false;
    }

	if (intersectionPoint) {
        // Get the intersection point.
        intersectionPoint->x = pA.x + ua*(pB.x - pA.x);
        intersectionPoint->y = pA.y + ua*(pB.y - pA.y);
    }

    return true;
}

//should be enhanced
bool IntersectionUtil::lineRectangle(const glm::vec2& pA1, const glm::vec2& pA2,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot, glm::vec2* intersectionPoint) {

    //NW
    glm::vec2 rectBNWPoint = rectBPos + glm::rotate(glm::vec2(- rectBSize.x, rectBSize.y) * .5f, rectBRot);
    //NE
    glm::vec2 rectBNEPoint = rectBPos + glm::rotate(glm::vec2(rectBSize.x, rectBSize.y) * .5f, rectBRot);
    //SW
    glm::vec2 rectBSWPoint = rectBPos + glm::rotate(glm::vec2(- rectBSize.x, - rectBSize.y) * .5f, rectBRot);
    //SE
    glm::vec2 rectBSEPoint = rectBPos + glm::rotate(glm::vec2(rectBSize.x, - rectBSize.y) * .5f, rectBRot);

    //try the 4 lines of the rectangle!
    return (lineLine(pA1, pA2, rectBNWPoint, rectBNEPoint, intersectionPoint)
            || lineLine(pA1, pA2, rectBNWPoint, rectBSWPoint, intersectionPoint)
            || lineLine(pA1, pA2, rectBNEPoint, rectBSEPoint, intersectionPoint)
            || lineLine(pA1, pA2, rectBSWPoint, rectBSEPoint, intersectionPoint));
}

bool IntersectionUtil::rectangleRectangle(const glm::vec2& rectAPos, const glm::vec2& rectASize, float rectARot,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot) {
    // quick out
    //~ if (Vector2::DistanceSquared(rectAPos, rectBPos) > pow(MathUtil::Max(rectASize.X, rectASize.Y) + MathUtil::Max(rectBSize.X, rectBSize.Y), 2)) {
    if (glm::distance(rectAPos, rectBPos) > glm::max(rectASize.x, rectASize.y) + glm::max(rectBSize.x, rectBSize.y)) {
        return false;
    }

    #define SIGN(x) ((x) > 0 ? 1:-1)
    const float coeff[] = {
        -0.5, 0.5,
        0.5, 0.5,
        0.5, -0.5,
        -0.5, -0.5
    };

    glm::vec2 aPoints[4];
    glm::vec2 bPoints[4];

    for (int i=0; i<4; i++) {
        aPoints[i] = rectAPos + glm::rotate(glm::vec2(rectASize.x * coeff[2*i], rectASize.y * coeff[2*i+1]), rectARot);
        bPoints[i] = rectBPos + glm::rotate(glm::vec2(rectBSize.x * coeff[2*i], rectBSize.y * coeff[2*i+1]), rectBRot);
    }

    // check A edges againts B points
    for (int i=0; i<4; i++) {
        glm::vec2 edge(aPoints[(i+1)%4] - aPoints[i]);
        float tmp = edge.x;
        edge.x = -edge.y;
        edge.y = tmp;

        bool success = true;
        int j, side = glm::sign(glm::dot(edge, bPoints[0] - aPoints[i]));
        for (j=1; success && j<4; j++) {
            int d = glm::sign(glm::dot(edge, bPoints[j] - aPoints[i]));
            success = (d == side);
        }
        // all points from B are on the same side
        // if at least one point of A is on the other side we're good
        if (success && j == 4) {
            int d1 = glm::sign(glm::dot(edge, aPoints[(i+2) % 4] - aPoints[i]));
            if (d1 != side)
                return false;
            int d2 = glm::sign(glm::dot(edge, aPoints[(i+3) % 4] - aPoints[i]));
            if (d2 != side)
                return false;
        }
    }

    // check B edges againts A points
    for (int i=0; i<4; i++) {
        glm::vec2 edge(bPoints[(i+1)%4] - bPoints[i]);
        float tmp = edge.x;
        edge.x = -edge.y;
        edge.y =tmp;

        bool success = true;
        int j, side = glm::sign(glm::dot(edge, aPoints[0] - bPoints[i]));
        for (j=1; success && j<4; j++) {
            int d = glm::sign(glm::dot(edge, aPoints[j] - bPoints[i]));
            success = (d == side);
        }
        // all points from A are on the same side
        // if at least one point of B is on the other side we're good
        if (success && j == 4) {
            int d1 = glm::sign(glm::dot(edge, bPoints[(i+2) % 4] - bPoints[i]));
            if (d1 != side)
                return false;
            int d2 = glm::sign(glm::dot(edge, bPoints[(i+3) % 4] - bPoints[i]));
            if (d2 != side)
                return false;
        }
    }
    return true;
}

bool IntersectionUtil::rectangleRectangle(const TransformationComponent* tc1, const TransformationComponent* tc2) {
    return rectangleRectangle(
        tc1->position, tc1->size, tc1->rotation,
        tc2->position, tc2->size, tc2->rotation);
}

bool IntersectionUtil::rectangleRectangle(const TransformationComponent* tc1,
    const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot) {
    return rectangleRectangle(
        tc1->position, tc1->size, tc1->rotation,
        rectBPos, rectBSize, rectBRot);
}
