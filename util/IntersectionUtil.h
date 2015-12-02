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

#include <glm/glm.hpp>
#include <tuple>
#include <vector>

struct TransformationComponent;
struct Polygon;

struct AABB {
    float left, right, top, bottom;
};

class IntersectionUtil {
    public:
    static bool
    pointLine(const glm::vec2& point, const glm::vec2& qA, const glm::vec2& qB);

    static bool pointRectangle(const glm::vec2& point,
                               const TransformationComponent* tc2);

    // rectPos is the center of the rectangle
    static bool pointRectangle(const glm::vec2& point,
                               const glm::vec2& rectPos,
                               const glm::vec2& rectSize,
                               float rectRotation);
    static bool pointRectangleAABB(const glm::vec2& point, const AABB& aabb);

    static bool lineCircle(const glm::vec2& pA,
                           const glm::vec2& pB,
                           const glm::vec2& center,
                           float radius);

    // Get the intersection point between SEGMENTS p and q. If IsStraigth is set
    // to true, consider the segment as a
    // STRAIGTH LINE: even if the intersection
    // point is not on the segment, accept it.
    static bool lineLine(const glm::vec2& pA,
                         const glm::vec2& pB,
                         const glm::vec2& qA,
                         const glm::vec2& qB,
                         glm::vec2* intersectionPoint,
                         bool pIsStraigth = false,
                         bool qIsStraigth = false);

    static int lineRectangle(const glm::vec2& pA1,
                             const glm::vec2& pA2,
                             const glm::vec2& rectBPos,
                             const glm::vec2& rectBSize,
                             float rectBRot,
                             glm::vec2* intersectionPoints,
                             glm::vec2* normalAtCollision = 0);

    static int linePolygon(const glm::vec2& pA1,
                           const glm::vec2& pA2,
                           const Polygon& p,
                           const glm::vec2& position,
                           const glm::vec2& size,
                           float rotation,
                           glm::vec2* intersectionPoints,
                           glm::vec2* normalAtCollision = 0);

    static int
    lineLines(const glm::vec2& pA1,
              const glm::vec2& pA2,
              const std::vector<std::tuple<glm::vec2, glm::vec2>> lines,
              glm::vec2* intersectionPoints,
              glm::vec2* normalAtCollision = 0);

    static bool rectangleRectangle(const glm::vec2& rectAPos,
                                   const glm::vec2& rectASize,
                                   float rectARot,
                                   const glm::vec2& rectBPos,
                                   const glm::vec2& rectBSize,
                                   float rectBRot);

    static bool rectangleRectangle(const TransformationComponent* tc1,
                                   const TransformationComponent* tc2);

    static int rectangleRectangle(const TransformationComponent* tc1,
                                  const TransformationComponent* tc2,
                                  glm::vec2* intersectionPoints,
                                  glm::vec2* normals);

    static bool rectangleRectangleAABB(const TransformationComponent* tc1,
                                       const TransformationComponent* tc2);

    static bool rectangleRectangleAABB(const AABB& a1, const AABB& a2);

    static bool rectangleRectangle(const TransformationComponent* tc1,
                                   const glm::vec2& rectBPos,
                                   const glm::vec2& rectBSize,
                                   float rectBRot);

    static void computeAABB(const TransformationComponent* tc,
                            AABB& aabb,
                            bool useRotation = true);
    static void computeAABB(const glm::vec2& position,
                            const glm::vec2& size,
                            float rotation,
                            AABB& aabb);
    static AABB mergeAABB(const AABB* aabb, int count);
};
