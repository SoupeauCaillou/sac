#pragma once

#include "../base/Vector2.h"
#include "../base/EntityManager.h"

class SteeringBehavior {
	public:
		struct WanderParams {
			float radius;
			float distance;
			float jitter;
			Vector2 target, debugTarget;
		};
	public:
		static Vector2 seek(Entity e, const Vector2& targetPos, float maxSpeed);

		static Vector2 flee(Entity e, const Vector2& targetPos, float maxSpeed);

		static Vector2 arrive(Entity e, const Vector2& targetPos, float maxSpeed, float deceleration);

        static Vector2 arrive(const Vector2& pos, const Vector2& linearVel,const Vector2& targetPos, float maxSpeed, float deceleration);

		static Vector2 wander(Entity e, WanderParams& params, float maxSpeed);
};
