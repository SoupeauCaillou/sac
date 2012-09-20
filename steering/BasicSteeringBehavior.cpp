#include "SteeringBehavior.h"
#include "../systems/TransformationSystem.h"
#include "../systems/PhysicsSystem.h"

Vector2 SteeringBehavior::seek(Entity e, const Vector2& targetPos, float maxSpeed) {
	Vector2 d (Vector2::Normalize(targetPos - TRANSFORM(e)->position) * maxSpeed);
	return d - PHYSICS(e)->linearVelocity;
}

Vector2 SteeringBehavior::flee(Entity e, const Vector2& targetPos, float maxSpeed) {
	Vector2 d (Vector2::Normalize(TRANSFORM(e)->position - targetPos) * maxSpeed);
	return d - PHYSICS(e)->linearVelocity;
}

Vector2 SteeringBehavior::arrive(Entity e, const Vector2& targetPos, float maxSpeed, float deceleration) {
	Vector2 toTarget (targetPos - TRANSFORM(e)->position);
	
	float d = toTarget.Normalize();
	if (d > 0) {
		float speed = MathUtil::Min(d / deceleration, maxSpeed);
		
		Vector2 desiredVelocity(toTarget * speed);
		return desiredVelocity - PHYSICS(e)->linearVelocity;
	}
	return Vector2::Zero;
}

Vector2 SteeringBehavior::wander(Entity e, WanderParams& params, float maxSpeed) {
	params.target += Vector2(
		MathUtil::RandomFloatInRange(-1.0f, 1.0f) * params.jitter,
		MathUtil::RandomFloatInRange(-1.0f, 1.0f) * params.jitter);
	params.target.Normalize();
	params.target *= params.radius;
	params.debugTarget = TRANSFORM(e)->position + Vector2::Rotate(Vector2(params.distance, 0) + params.target, TRANSFORM(e)->rotation);
	
	return seek(e, params.debugTarget, maxSpeed);
}
