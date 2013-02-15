#include "DebuggingSystem.h"
#include "base/EntityManager.h"
#include "base/Log.h"
#include "base/Assert.h"
#include "base/PlacementHelper.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/AutonomousAgentSystem.h"

INSTANCE_IMPL(DebuggingSystem);


DebuggingSystem::DebuggingSystem() : ComponentSystemImpl<DebuggingComponent>("Debugging") {
	DebuggingComponent dc;
	activate = true;
}

static Entity init() {
		Entity e = theEntityManager.CreateEntity();

      ADD_COMPONENT(e, Transformation);
      ADD_COMPONENT(e, Rendering);

		LOGI("DEBUG SYSTEM: %ld created", e);
      return e;
}

void DebuggingSystem::switchActivate(bool value) {
	activate = !value;
	switchActivate();
}

void DebuggingSystem::switchActivate() {
	for (unsigned index = 0; index < entities.size(); ++index) {
		//check its z is not null (z = 0 <-> unused entities)
		if (activate || (!activate && TRANSFORM(entities[index])->z))
			RENDERING(entities[index])->hide = activate;
	}
	//doing it after because hide = !show..
	activate = !activate;

	//- LOGI("activate switched to %d", activate);
}

static int setSegmentEntity(Entity e, Entity from, Entity to, const Color & c) {
	Vector2 fromV = TRANSFORM(from)->worldPosition;
	Vector2 toV = TRANSFORM(to)->worldPosition;


	//don't draw segment if Entity need to cross edges
	if (Vector2::edgeToCross(fromV, toV) != 0) {
		return 0;
	}




	RENDERING(e)->color = c;
	RENDERING(e)->color.a = 0.5;
	TRANSFORM(e)->parent = 0;
	RENDERING(e)->hide = false;
	TRANSFORM(e)->position = (toV + fromV) / 2.;

	Vector2 between = toV - fromV;

	if (!between.LengthSquared()) {
		/*
		Hack, it's because update is the 'wrong order':
		rendering -> debug -> transformation, so transformation hasn't calculed the
		world position yet (mainly for the first frame), and so it returns
		Vector2::Zero; currently we'll assume these entities haven't any
		parent, so worldPosition == position.
		@todo : Must be fixed.
		*/
		ASSERT(!TRANSFORM(to)->parent, "This hack don't work with parented entities");

		between = - TRANSFORM(from)->position + TRANSFORM(to)->position;
		assert(between.LengthSquared());
	}

	TRANSFORM(e)->size = Vector2(0.05, between.Length());
	TRANSFORM(e)->rotation = MathUtil::AngleFromVector(between) + MathUtil::PiOver2;

	return 1;
}
void DebuggingSystem::addEntities() {
	//if we have less entities than required..
	for (int i = 0; i < 4; ++i) {
		entities.push_back(init());
	}
}
void DebuggingSystem::DoUpdate(float) {
	if (!activate) return;

	unsigned index;

	index = 0;
	FOR_EACH_ENTITY_COMPONENT(Debugging, entity, dc)
		if (entities.size() - index < 4) {
			addEntities();
		}
		if (dc->showLinked) {
			if (TRANSFORM(entity)->parent) {
				//- LOGI("parent debug");
				int count = setSegmentEntity(entities[index], entity, TRANSFORM(entity)->parent, Color(0,1,0));
				for (int i = 0; i < count; ++i) TRANSFORM(entities[index + i])->z = 0.01;
				index+=count;
			}
			if (AUTONOMOUS(entity)->fleeTarget) {
				//- LOGI("flee debug");
				int count = setSegmentEntity(entities[index], entity, AUTONOMOUS(entity)->fleeTarget, Color(0, 1, 1));
				for (int i = 0; i < count; ++i) TRANSFORM(entities[index + i])->z = 0.03;
				index+=count;
			}

			if (AUTONOMOUS(entity)->arriveTarget) {
				//- LOGI("arrive debug");
				int count = setSegmentEntity(entities[index], entity, AUTONOMOUS(entity)->arriveTarget, Color(0, 0, 1));
				for (int i = 0; i < count; ++i) TRANSFORM(entities[index + i])->z = 0.02;
				index+=count;
			}
		}

		if (dc->showHighligh) {
	      RENDERING(entities[index])->color = Color(1,0,0,.5);
			TRANSFORM(entities[index])->parent = entity;

			TRANSFORM(entities[index])->size = TRANSFORM(entity)->size * 3;
	      TRANSFORM(entities[index])->z = 1. - TRANSFORM(entity)->z;

			TRANSFORM(entities[index])->position = Vector2::Zero;
			TRANSFORM(entities[index])->rotation = 0.;

			RENDERING(entities[index])->hide = false;

			++index;
		}
	}

	while (index < entities.size()) {
		Entity e = entities[index];

		TRANSFORM(e)->parent = 0;
		TRANSFORM(e)->position = Vector2::Zero;
		TRANSFORM(e)->size = Vector2::One;
		TRANSFORM(e)->z = 0;

		RENDERING(e)->hide = true;

		++index;
	}
}
