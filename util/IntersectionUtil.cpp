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



#include "IntersectionUtil.h"

#include "systems/TransformationSystem.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>
#include <alloca.h>

#if SAC_DEBUG && !DISABLE_COLLISION_SYSTEM
#include "util/Draw.h"
#include "systems/CollisionSystem.h"
#endif

//never trust floats, they're evils
const float eps = 0.0001f;

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

bool IntersectionUtil::lineCircle(const glm::vec2& _pA, const glm::vec2& _pB, const glm::vec2& center, float radius) {
    const auto pA(_pA - center);
    const auto pB(_pB - center);
    // from http://mathworld.wolfram.com/Circle-LineIntersection.html
    float d2 = glm::distance2(pA, pB);
    float D = pA.x * pB.y - pB.x * pA.y;
    float discriminant = radius * radius * d2 - D * D;
    return discriminant >= 0;
}

bool IntersectionUtil::pointRectangleAABB(const glm::vec2& p, const AABB& aabb) {
    return !(
        p.x < aabb.left ||
        p.x > aabb.right ||
        p.y < aabb.bottom ||
        p.y > aabb.top);
}

bool IntersectionUtil::pointRectangle(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectSize, float rectRotation) {
    glm::vec2 p(point - rectPos);
    if (rectRotation != 0.0) {
        p = (glm::rotate(p, -rectRotation));
    }

    return (glm::abs(p.x) < rectSize.x * 0.5 &&
        glm::abs(p.y) < rectSize.y * 0.5);
}

// from http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
bool IntersectionUtil::lineLine(const glm::vec2& pA, const glm::vec2& pB,
            const glm::vec2& qA, const glm::vec2& qB, glm::vec2* intersectionPoint, bool pIsStraigth, bool qIsStraigth) {
    const float denom = ((qB.y - qA.y)*(pB.x - pA.x)) -
                      ((qB.x - qA.x)*(pB.y - pA.y));

    const float nume_a = ((qB.x - qA.x)*(pA.y - qA.y)) -
                   ((qB.y - qA.y)*(pA.x - qA.x));

    const float nume_b = ((pB.x - pA.x)*(pA.y - qA.y)) -
                   ((pB.y - pA.y)*(pA.x - qA.x));

    //line are parallels - need to check if they are coincidents or not
    if(glm::abs(denom) < eps)
    {
        // they are coincidents if one of the extremity of the smaller line is contained in the big one

        glm::vec2 smallerA(pA), smallerB(pB);
        glm::vec2 biggerA(qA), biggerB(qB);
        if (glm::length2(pB - pA) > glm::length2(qB - qA)) {
            smallerA = qA;
            smallerB = qB;
            biggerA = pA;
            biggerB = pB;
        }

        const float pDeltaX = biggerB.x - biggerA.x;
        const float pDeltaY = biggerB.y - biggerA.y;

        //if the line is not totally VERTICALE, use the normal approach
        if (glm::abs(pDeltaX) > eps) {
            const float ua = (smallerA.x - biggerA.x) / pDeltaX;
            const float ub = (smallerB.x - biggerA.x) / pDeltaX;

            //is smallerA in segment?
            if ((pIsStraigth || (ua >= -eps && ua <= 1.f + eps)) && glm::abs(smallerA.y - (biggerA.y + ua * pDeltaY)) < eps) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = biggerA + (biggerB - biggerA) * ua;
                }
                return true;
            // or smallerB?
            } else if ((pIsStraigth || (ub >= -eps && ub <= 1.f + eps)) && glm::abs(smallerB.y - (biggerA.y + ub * pDeltaY)) < eps) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = biggerA + (biggerB - biggerA) * ub;
                }
                return true;
            }
            // LOGI(std::fixed << std::setprecision(5) << "1. " << ua << " " << ub);
        //vertical line, special case. Just need to check if they are on the same X
        } else if (glm::abs(smallerA.x - biggerA.x) < eps) {
            const float ua = (smallerA.y - biggerA.y) / pDeltaY;
            const float ub = (smallerB.y - biggerA.y) / pDeltaY;

            if (pIsStraigth || (ua >= -eps && ua <= 1.f + eps)) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = biggerA + (biggerB - biggerA) * ua;
                }
                return true;
            } else if (pIsStraigth || (ub >= -eps && ub <= 1.f + eps)) {
                if (intersectionPoint != 0) {
                    *intersectionPoint = biggerA + (biggerB - biggerA) * ub;
                }
                return true;
            }
            // LOGI(std::fixed << std::setprecision(5) << "2. " << ua << " " << ub);
        }
        // LOGI(std::fixed << std::setprecision(5) << denom << " " << nume_a << " " << nume_b << " " << pDeltaX << " " << pDeltaY);
        return false;
    }

    const float inv_denom = 1.0f / denom;
    const float ua = nume_a * inv_denom;
    const float ub = nume_b * inv_denom;

    // LOGI(std::fixed << std::setprecision(5) << "3. " << ua << " " << ub);
    // LOGI(std::fixed << std::setprecision(5) << denom << " " << nume_a << " " << nume_b);

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

static glm::vec2 produceNormal(const glm::vec2& start, const glm::vec2& end) {
    glm::vec2 n (glm::normalize(end - start));
    return glm::vec2(-n.y, n.x);
}

//should be enhanced
int IntersectionUtil::lineRectangle(const glm::vec2& pA1, const glm::vec2& pA2,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot, glm::vec2* intersectionPoint, glm::vec2* normalAtCollision) {

    return linePolygon(pA1, pA2, theTransformationSystem.shapes[Shape::Square], rectBPos, rectBSize, rectBRot, intersectionPoint, normalAtCollision);
}

int IntersectionUtil::linePolygon(const glm::vec2& pA1, const glm::vec2& pA2,
            const Polygon& p, const glm::vec2& position, const glm::vec2& size, float rotation, glm::vec2* intersectionPoints, glm::vec2* normalAtCollision) {

    std::vector<std::tuple<glm::vec2, glm::vec2>> lines;

    // transform pA1/pA2 in Polygon base
    const auto t1 = glm::rotate((pA1 - position), -rotation) / size;
    const auto t2 = glm::rotate((pA2 - position), -rotation) / size;

    const unsigned count = p.vertices.size();
    for (unsigned i=0; i<count; ++i) {
        lines.push_back(std::make_tuple(p.vertices[i], p.vertices[(i + 1) % count]));
    }

    int intersections = lineLines(t1, t2, lines, intersectionPoints, normalAtCollision);

    if (intersections) {
        for (int i=0; i<intersections; i++) {
            if (intersectionPoints)
                intersectionPoints[i] = position + glm::rotate(intersectionPoints[i] * size, rotation);
            if (normalAtCollision)
                normalAtCollision[i] = glm::rotate(normalAtCollision[i], rotation);
        }
    }

    return intersections;
}

int IntersectionUtil::lineLines(const glm::vec2& pA1, const glm::vec2& pA2, const std::vector<std::tuple<glm::vec2, glm::vec2>> lines, glm::vec2* intersectionPoints, glm::vec2* normalAtCollision) {
    glm::vec2 intersection;
    int count = 0;
    float* distances = (float*)alloca(sizeof(float) * lines.size());
    const float EPSILON = 0.0001f;

    for (const auto& line: lines) {
        const auto& pB1 (std::get<0>(line));
        const auto& pB2 (std::get<1>(line));
        if (lineLine(pA1, pA2, pB1, pB2, &intersection)) {
            float d = distances[count] = glm::distance2(intersection, pA1);
            bool ignore = false;

            for (int i=0; i<count; i++) {
                ignore |= (glm::abs(distances[i] - d) < EPSILON);
            }

            if (!ignore) {
                if (intersectionPoints) {
                    intersectionPoints[count] = intersection;
                }
                if (normalAtCollision) {
                    normalAtCollision[count] = produceNormal(pB1, pB2);
#if SAC_DEBUG && !DISABLE_COLLISION_SYSTEM
                    if (theCollisionSystem.showDebug) {
                        Draw::Point(pB1, Color(1, 0, 0));
                        Draw::Point(pB2, Color(0, 0, 1));
                    }
#endif
                }
                count++;
            }
        }
    }

    return count;
}

void IntersectionUtil::computeAABB(const TransformationComponent* tc, AABB& aabb, bool useRotation) {
    computeAABB(tc->position, tc->size, useRotation ? tc->rotation : 0.0f, aabb);
}

void IntersectionUtil::computeAABB(const glm::vec2& position, const glm::vec2& size, float rotation, AABB& aabb) {
    glm::vec2 halfsize(0.0f);
    if (glm::abs(rotation) < 0.001) {
        halfsize = size * 0.5f;
    } else {
        glm::vec2 p = size * glm::vec2(0.5, 0.5);
        glm::vec2 q = size * glm::vec2(0.5, -0.5);
        p = glm::rotate(p, rotation);
        q = glm::rotate(q, rotation);
        halfsize.x = glm::max(glm::abs(p.x), glm::abs(q.x));
        halfsize.y = glm::max(glm::abs(p.y), glm::abs(q.y));
    }
    aabb.left = position.x - halfsize.x;
    aabb.right = position.x + halfsize.x;
    aabb.bottom = position.y - halfsize.y;
    aabb.top = position.y + halfsize.y;
}

AABB IntersectionUtil::mergeAABB(const AABB* aabb, int count) {
    LOGF_IF(count <= 0, "count should be > 0");
    AABB result = aabb[0];
    for (int i=1; i<count; i++) {
        result.left = glm::min(result.left, aabb[i].left);
        result.right = glm::max(result.right, aabb[i].right);
        result.bottom = glm::min(result.bottom, aabb[i].bottom);
        result.top = glm::max(result.top, aabb[i].top);
    }
    return result;
}

bool IntersectionUtil::rectangleRectangleAABB(const TransformationComponent* tc1, const TransformationComponent* tc2) {
    AABB a1, a2;
    computeAABB(tc1, a1);
    computeAABB(tc2, a2);

    return rectangleRectangleAABB(a1, a2);
}

bool IntersectionUtil::rectangleRectangleAABB(const AABB& a1, const AABB& a2) {
    return !(
        a1.right < a2.left ||
        a2.right < a1.left ||
        a1.top < a2.bottom ||
        a2.top < a1.bottom);
}

static void computeRectangleVertices(const glm::vec2& rectAPos, const glm::vec2& rectASize, float rectARot, glm::vec2* out) {
    const static float coeff[] = {
        -0.5, 0.5, // top-left
        0.5, 0.5,  // top-right
        0.5, -0.5,
        -0.5, -0.5
    };

    glm::vec2* ptr = out;
    for (int i=0; i<2; i++) {
        *out++ =
            glm::rotate(
                glm::vec2(
                    rectASize.x * coeff[2*i],
                    rectASize.y * coeff[2*i+1]),
                rectARot);
    }

    ptr[2] = rectAPos - ptr[0];
    ptr[0] += rectAPos;
    ptr[3] = rectAPos - ptr[1];
    ptr[1] += rectAPos;
}

bool IntersectionUtil::rectangleRectangle(const glm::vec2& rectAPos, const glm::vec2& rectASize, float rectARot,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot) {
    // quick out
    //~ if (Vector2::DistanceSquared(rectAPos, rectBPos) > pow(MathUtil::Max(rectASize.X, rectASize.Y) + MathUtil::Max(rectBSize.X, rectBSize.Y), 2)) {
    if (glm::distance2(rectAPos, rectBPos) > glm::pow(glm::max(rectASize.x, rectASize.y) + glm::max(rectBSize.x, rectBSize.y), 2.0f)) {
        return false;
    }

    #define SIGN(x) ((x) > 0 ? 1:-1)
    glm::vec2 aPoints[4];
    computeRectangleVertices(rectAPos, rectASize, rectARot, aPoints);
    glm::vec2 bPoints[4];
    computeRectangleVertices(rectBPos, rectBSize, rectBRot, bPoints);

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
    if (glm::abs(tc1->rotation) < 0.001 && glm::abs(tc2->rotation) < 0.001)
        return rectangleRectangleAABB(tc1, tc2);

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

int IntersectionUtil::rectangleRectangle(
        const TransformationComponent* tc1,
        const TransformationComponent* tc2,
        glm::vec2* intersectionPoints,
        glm::vec2* normals) {
    glm::vec2 aPoints[4];
    computeRectangleVertices(tc1->position, tc1->size, tc1->rotation, aPoints);
    glm::vec2 bPoints[4];
    computeRectangleVertices(tc2->position, tc2->size, tc2->rotation, bPoints);

    std::vector<std::tuple<glm::vec2, glm::vec2>> bLines;
    bLines.resize(4);
    for (int i=0; i<4; i++) {
        bLines[i] = std::make_tuple(bPoints[i], bPoints[(i + 1) % 4]);
    }

    int count = 0;
    for (int i=0; i<4; i++) {
        count += lineLines(aPoints[i], aPoints[(i+1) % 4], bLines, intersectionPoints + count, normals + count);
    }

    bool inside[4] = { false, false, false, false };
    for (int i=0; i<4; i++) {
        inside[i] = pointRectangle(bPoints[i], tc1);
    }
    for (int i=0; i<4 && count < 4; i++) {
        // B edge in A edge
        if (inside[i] && inside[(i+1)%4]) {
            if (intersectionPoints) {
                intersectionPoints[count] = (bPoints[i] + bPoints[(i + 1) % 4]) * 0.5f;
            }
            if (normals) {
                normals[count] = produceNormal(bPoints[i], bPoints[(i + 1) % 4]);
            }
            count++;
        }
    }
    return count;
}
