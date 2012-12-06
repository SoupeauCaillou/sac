#pragma once

#include "../base/Vector2.h"
class TransformationComponent;

class IntersectionUtil {
	public:
		static bool pointRectangle(const Vector2& point, const Vector2& rectPos, const Vector2& rectSize);

		static bool lineLine(const Vector2& pA, const Vector2& pB, const Vector2& qA, const Vector2& qB, Vector2* intersectionPoint);

        static bool rectangleRectangle(const Vector2& rectAPos, const Vector2& rectASize, float rectARot,
            const Vector2& rectBPos, const Vector2& rectBSize, float rectBRot);

        static bool rectangleRectangle(const TransformationComponent* tc1, const TransformationComponent* tc2);
};
