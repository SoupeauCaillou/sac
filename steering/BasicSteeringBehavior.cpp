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
	return arrive(TRANSFORM(e)->position, PHYSICS(e)->linearVelocity, targetPos, maxSpeed, deceleration);
}

Vector2 SteeringBehavior::arrive(const Vector2& pos, const Vector2& linearVel,const Vector2& targetPos, float maxSpeed, float deceleration) {
    Vector2 toTarget (targetPos - pos);
    
    float d = toTarget.Normalize();
    if (d > 0) {
        float speed = MathUtil::Min(d / deceleration, maxSpeed);
        Vector2 desiredVelocity(toTarget * speed);
        return desiredVelocity - linearVel;
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
