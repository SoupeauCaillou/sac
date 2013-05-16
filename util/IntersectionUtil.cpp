#include "IntersectionUtil.h"
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "../systems/TransformationSystem.h"
#include <cmath>

bool IntersectionUtil::pointRectangle(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectSize) {
	return (glm::abs(rectPos.x - point.x) < rectSize.x * 0.5 &&
		glm::abs(rectPos.y - point.y) < rectSize.y * 0.5);
}

// from http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
bool IntersectionUtil::lineLine(const glm::vec2& pA, const glm::vec2& pB, const glm::vec2& qA, const glm::vec2& qB, glm::vec2* intersectionPoint) {
	float denom = ((qB.y - qA.y)*(pB.x - pA.x)) -
                      ((qB.x - qA.x)*(pB.y - pA.y));

    float nume_a = ((qB.x - qA.x)*(pA.y - qA.y)) -
                   ((qB.y - qA.y)*(pA.x - qA.x));

    float nume_b = ((pB.x - pA.x)*(pA.y - qA.y)) -
                   ((pB.y - pA.y)*(pA.x - qA.x));

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
            intersectionPoint->x = pA.x + ua*(pB.x - pA.x);
            intersectionPoint->y = pA.y + ua*(pB.y - pA.y);
        }

        return true;
    }

    return false;
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
