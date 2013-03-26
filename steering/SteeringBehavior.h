#pragma once

#include <glm/glm.hpp>
#include "../base/EntityManager.h"

class SteeringBehavior {
	public:
		struct WanderParams {
			float radius;
			float distance;
			float jitter;
			glm::vec2 target, debugTarget;
		};
	public:
		static glm::vec2 seek(Entity e, const glm::vec2& targetPos, float maxSpeed);

		static glm::vec2 flee(Entity e, const glm::vec2& targetPos, float maxSpeed);

		static glm::vec2 arrive(Entity e, const glm::vec2& targetPos, float maxSpeed, float deceleration);

        static glm::vec2 arrive(const glm::vec2& pos, const glm::vec2& linearVel,const glm::vec2& targetPos, float maxSpeed, float deceleration);

		static glm::vec2 wander(Entity e, WanderParams& params, float maxSpeed);
};
