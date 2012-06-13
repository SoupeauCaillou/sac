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
#include "MorphingSystem.h"

INSTANCE_IMPL(MorphingSystem);

MorphingSystem::MorphingSystem() : ComponentSystemImpl<MorphingComponent>("Morphing") { }

void MorphingSystem::DoUpdate(float dt) {
	for(std::map<Entity, MorphingComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
		MorphingComponent* m = (*jt).second;

		if (!m->active || m->activationTime>m->timing) {
			m->active = false;
			m->activationTime = 0;
            for (unsigned int i=0; i<m->elements.size(); i++)
                m->elements[i]->ended = false;
			continue;
		}
		if (m->active) {
			m->activationTime += dt;
			m->value = MathUtil::Min(1.0f, m->activationTime/m->timing);
            for (unsigned int i=0; i<m->elements.size(); i++) {
                if (!m->elements[i]->ended) {
					m->elements[i]->lerp(MathUtil::Min(1.0f, m->value * m->elements[i]->coeff));
					if (m->value == 1) {
						m->elements[i]->ended = true;
					}
				}
            }
		}
	}
}

void MorphingSystem::reverse(MorphingComponent* mc) {
	for (unsigned int i=0; i<mc->elements.size(); i++) {
		mc->elements[i]->reverse();
	}
}

void MorphingSystem::clear(MorphingComponent* mc) {
	for (unsigned int i=0; i<mc->elements.size(); i++) {
		delete mc->elements[i];
	}
	mc->elements.clear();
}
