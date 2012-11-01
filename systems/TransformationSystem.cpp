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
#include "TransformationSystem.h"

INSTANCE_IMPL(TransformationSystem);

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") {
    TransformationComponent tc;
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(position.X, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(position.Y, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(size.X, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(size.Y, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(rotation, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(z, tc), 0.001));
}

void TransformationSystem::DoUpdate(float dt __attribute__((unused))) {
	//update orphans first
    FOR_EACH_ENTITY_COMPONENT(Transformation, entity, bc)
		if (!bc->parent) {
			bc->worldPosition = bc->position;
			bc->worldRotation = bc->rotation;
            bc->worldZ = bc->z;
		}
	}
	//copy parent property to its sons
    FOR_EACH_ENTITY_COMPONENT(Transformation, a, bc)
		Entity parent = bc->parent;
		if (parent) {
			while (TRANSFORM(parent)->parent) {
				parent = TRANSFORM(parent)->parent;
			}
			const TransformationComponent* pbc = TRANSFORM(bc->parent);
			bc->worldPosition = pbc->worldPosition + Vector2::Rotate(bc->position, pbc->worldRotation);
			bc->worldRotation = pbc->worldRotation + bc->rotation;
            bc->worldZ = pbc->worldZ + bc->z;
		}
    }
}

void TransformationSystem::setPosition(TransformationComponent* tc, const Vector2& p, PositionReference ref) {
	// x
	switch (ref) {
		case NW:
		case W:
		case SW:
			tc->position.X = p.X + tc->size.X * 0.5;
			break;
		case N:
		case C:
		case S:
			tc->position.X = p.X;
			break;
		case NE:
		case E:
		case SE:
			tc->position.X = p.X - tc->size.X * 0.5;
			break;
	}
	// y
	switch (ref) {
		case NW:
		case N:
		case NE:
			tc->position.Y = p.Y - tc->size.Y * 0.5;
			break;
		case W:
		case C:
		case E:
			tc->position.Y = p.Y;
			break;
		case SW:
		case S:
		case SE:
			tc->position.Y = p.Y + tc->size.Y * 0.5;
			break;
	}
}
