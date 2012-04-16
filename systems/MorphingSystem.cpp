#include "MorphingSystem.h"

INSTANCE_IMPL(MorphingSystem);

MorphingSystem::MorphingSystem() : ComponentSystemImpl<MorphingComponent>("Morphing") { }

void MorphingSystem::DoUpdate(float dt) {
	for(std::map<Entity, MorphingComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
		MorphingComponent* m = (*jt).second;

		if ((!m->active && m->activationTime<=0) || m->activationTime>m->timing) {
			m->active = false;
			m->activationTime = 0;
			continue;
		}
		if (m->active) {
			m->activationTime += dt;
			m->value = m->activationTime/m->timing;
			m->pos = MathUtil::Lerp(m->from, m->to, m->value);
		}
	}
}

