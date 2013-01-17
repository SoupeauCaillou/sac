/*!
 * \file IntersectionUtil.h
 * \brief A tool to check intersection between entity
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include "../base/Vector2.h"
struct TransformationComponent;

/*!
 * \class IntesectionUtil
 * \brief 
 */
class IntersectionUtil {
	public:
        /*! Checks if a point is in a rectangle or not
         * \param point : point coordinates
         * \param rectPos : Position of the rectangle
         * \param rectSize : Size of the rectangle
         * \return Return "true" if point is in rectangle. Otherwise, it's return "false" */
		static bool pointRectangle(const Vector2& point, const Vector2& rectPos, const Vector2& rectSize);

        /*! Checks if a line crosses another line
         *  \param pA : first point of line p
         *  \param pB : second point of line p
         *  \param qA : first point of line q
         *  \param qB : second point of line q
         *  \param [out] intersectionPoint : Position of intersection (NULL if returned value is "false")
         *  \return Return "true" if lines are crossing. Otherwise, it's return "false" */
        static bool lineLine(const Vector2& pA, const Vector2& pB, const Vector2& qA, const Vector2& qB, Vector2* intersectionPoint);

        /*! Checks if 2 rectangles have similar points
         *  \param rectAPos : Position of the rectangle A
         *  \param rectASize : Size of the rectangle A
         *  \param rectARot : Angle of the rectangle A relative to horizontal (in radians)
         *  \param rectBPos : Position of the rectangle B
         *  \param rectBSize : Size of the rectangle B
         *  \param rectBRot : Angle of the rectangle B relative to horizontal (in radians)
         *  \return Return "true" if 2 rectangles have similar points. Otherwise, it's return "false" */
        static bool rectangleRectangle(const Vector2& rectAPos, const Vector2& rectASize, float rectARot,
            const Vector2& rectBPos, const Vector2& rectBSize, float rectBRot);

        /*! Checks if 2 rectangles have similar points
         *  \param tc1 : rectangle A
         *  \param tc2 : rectangle B
         *  \return Return "true" if 2 rectangles have similar points. Otherwise, it's return "false" */
        static bool rectangleRectangle(const TransformationComponent* tc1, const TransformationComponent* tc2);
};
