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
#pragma once

#include "base/Vector2.h"
#include "base/MathUtil.h"

#include "System.h"


struct TransformationComponent {
	TransformationComponent(): position(Vector2::Zero), size(1.0f, 1.0f), rotation(0), z(0), parent(0) { }
	
	Vector2 position, worldPosition, size;
	float rotation, worldRotation;//radians
	float z, worldZ;

	Entity parent;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)

public:
	enum PositionReference {
		NW, N, NE,
		W , C, E ,
		SW, S, SE
	};
	static void setPosition(TransformationComponent* tc, const Vector2& p, PositionReference ref=C);
};
