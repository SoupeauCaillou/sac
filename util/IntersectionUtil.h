#pragma once

#include <glm/glm.hpp>
struct TransformationComponent;

class IntersectionUtil {
	public:
		static bool pointRectangle(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectSize);

        static bool lineLine(const glm::vec2& pA, const glm::vec2& pB,
            const glm::vec2& qA, const glm::vec2& qB, glm::vec2* intersectionPoint);

        static bool lineRectangle(const glm::vec2& pA1, const glm::vec2& pA2,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot, glm::vec2* intersectionPoint);

        static bool rectangleRectangle(const glm::vec2& rectAPos, const glm::vec2& rectASize, float rectARot,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot);

        static bool rectangleRectangle(const TransformationComponent* tc1,
            const TransformationComponent* tc2);

        static bool rectangleRectangle(const TransformationComponent* tc1,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot);
};
