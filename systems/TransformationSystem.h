/*!
 * \file TransformationSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include "base/Vector2.h"

#include "System.h"

/*!
 * \struct TransformationComponent
 * \brief
 */
struct TransformationComponent {
	TransformationComponent(): position(Vector2::Zero), size(1.0f, 1.0f), rotation(0), z(0), parent(0) { }

	Vector2 position, worldPosition, size; /*!< */
	float rotation, worldRotation; /*!< (radians)*/
	float z, worldZ; /*!< */

	Entity parent; /*!< */
};

/*! \def
 *  \brief
 */
#define theTransformationSystem TransformationSystem::GetInstance()

/*! \def
 *  \brief
 */
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)

public:
    /*! \enum PositionReference
     *  \brief */
	enum PositionReference {
		NW, N, NE, /*! North-West, North, North-East */
		W , C, E , /*! West, Center, East */
		SW, S, SE /*! South-West, South, South-East */
	};

    /*! \brief
     *  \param tc :
     *  \param p :
     *  \param ref : (by default : C) */
	static void setPosition(TransformationComponent* tc, const Vector2& p, PositionReference ref=C);

	#ifdef DEBUG
	void preDeletionCheck(Entity e) {
		FOR_EACH_COMPONENT(Transformation, bc)
			if (bc->parent == e) {
				LOGE("deleting an entity which is parent ! (Entity %ld)", e);
			}
		}
	}
	#endif
};
