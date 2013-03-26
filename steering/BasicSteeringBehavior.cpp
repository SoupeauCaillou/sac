#include "SteeringBehavior.h"
#include "../systems/TransformationSystem.h"
#include "../systems/PhysicsSystem.h"
#include <glm/gtx/rotate_vector.hpp>

glm::vec2 SteeringBehavior::seek(Entity e, const glm::vec2& targetPos, float maxSpeed) {
	glm::vec2 d (glm::normalize(targetPos - TRANSFORM(e)->position) * maxSpeed);
	return d - PHYSICS(e)->linearVelocity;
}

glm::vec2 SteeringBehavior::flee(Entity e, const glm::vec2& targetPos, float maxSpeed) {
	glm::vec2 d (glm::normalize(TRANSFORM(e)->position - targetPos) * maxSpeed);
	return d - PHYSICS(e)->linearVelocity;
}

glm::vec2 SteeringBehavior::arrive(Entity e, const glm::vec2& targetPos, float maxSpeed, float deceleration) {
	return arrive(TRANSFORM(e)->position, PHYSICS(e)->linearVelocity, targetPos, maxSpeed, deceleration);
}

glm::vec2 SteeringBehavior::arrive(const glm::vec2& pos, const glm::vec2& linearVel,const glm::vec2& targetPos, float maxSpeed, float deceleration) {
    glm::vec2 toTarget (targetPos - pos);
    float d = glm::length(toTarget); 
    toTarget = glm::normalize(toTarget);
    
    if (d > 0) {
        float speed = glm::min(d / deceleration, maxSpeed);
        glm::vec2 desiredVelocity(toTarget * speed);
        return desiredVelocity - linearVel;
    }
    return glm::vec2(0.0f, 0.0f);
}

glm::vec2 SteeringBehavior::wander(Entity e, WanderParams& params, float maxSpeed) {
	params.target += glm::vec2(
		glm::linearRand(-1.0f, 1.0f) * params.jitter,
		glm::linearRand(-1.0f, 1.0f) * params.jitter);
	params.target = glm::normalize(params.target);
	params.target *= params.radius;
	params.debugTarget = TRANSFORM(e)->position + glm::rotate(glm::vec2(params.distance, 0.0f) + params.target, TRANSFORM(e)->rotation);
	
	return seek(e, params.debugTarget, maxSpeed);
}
