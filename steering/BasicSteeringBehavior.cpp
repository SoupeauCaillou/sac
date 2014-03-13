/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "SteeringBehavior.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/RenderingSystem.h"

#include <util/IntersectionUtil.h>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

#if SAC_DEBUG
#include "util/DrawSomething.h"
#endif

glm::vec2 SteeringBehavior::seek(Entity e, const glm::vec2& targetPos, float maxSpeed) {
    return seek(TRANSFORM(e)->position, PHYSICS(e)->linearVelocity, targetPos, maxSpeed);
}

glm::vec2 SteeringBehavior::seek(const glm::vec2& pos, const glm::vec2& linearVel, const glm::vec2& targetPos, float maxSpeed) {
    glm::vec2 toTarget (targetPos - pos);
    float d = glm::length(toTarget); 
    
    if (d > 0) {
        return toTarget * (maxSpeed / d) - linearVel;
    } else {
        return glm::vec2(.0f);
    }
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
    
    if (d > 0) {
        toTarget = glm::normalize(toTarget);
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


#define BASIC_STEERING_GRAPHICAL_DEBUG 1
glm::vec2 SteeringBehavior::avoid(Entity e, const glm::vec2& velocity, std::list<Entity>& obstacles, float maxSpeed) {
    float size = TRANSFORM(e)->size.x * (1 + 0.5 * glm::length(velocity) / maxSpeed);
    const glm::vec2 & rectSize = glm::vec2(size, TRANSFORM(e)->size.y);
    const glm::vec2 & rectPos = TRANSFORM(e)->position + glm::rotate(glm::vec2(rectSize.x * 0.5, 0), TRANSFORM(e)->rotation);
    float rectRot = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(velocity));

    #if BASIC_STEERING_GRAPHICAL_DEBUG
        DrawSomething::DrawPointRestart("basicsteeringbehavior");
        DrawSomething::DrawRectangleRestart("basicsteeringbehavior");
        DrawSomething::DrawVec2Restart("basicsteeringbehavior");

        // display box-view of the object (where it wants to go)
        DrawSomething::DrawRectangle("basicsteeringbehavior", rectPos, rectSize, rectRot, Color(1, 0, 0, .5));
    #endif

    glm::vec2 intersectionPoints[4], normals[4];
    const float halfWidth = TRANSFORM(e)->size.y * 0.5;
    glm::vec2 nearest, normal;
    Entity obs;
    float minDist = 1000;
    for (auto obstacle : obstacles) {
        auto & pos = TRANSFORM(obstacle)->position;

        if (IntersectionUtil::rectangleRectangle(pos, TRANSFORM(obstacle)->size, 
            TRANSFORM(obstacle)->rotation, rectPos, rectSize, rectRot)) {

            #if BASIC_STEERING_GRAPHICAL_DEBUG
            // display a box containing the obstacle
            DrawSomething::DrawRectangle("basicsteeringbehavior", pos, 
                TRANSFORM(obstacle)->size + glm::vec2(halfWidth), TRANSFORM(obstacle)->rotation,
                Color(0, 1, 0, .5));
            #endif

            // we need to get the point of intersection of them to know if its the
            // closer rectangle from entity e
            int intersectCount = IntersectionUtil::lineRectangle(
                // open-ended line starting at e's position
                TRANSFORM(e)->position, TRANSFORM(e)->position + glm::rotate(glm::vec2(1000, 0), TRANSFORM(e)->rotation),
                // rectangle
                pos, TRANSFORM(obstacle)->size + glm::vec2(halfWidth), TRANSFORM(obstacle)->rotation,
                // result
                intersectionPoints, normals);
            
            for (int i = 0; i < intersectCount; ++i) {
                #if BASIC_STEERING_GRAPHICAL_DEBUG
                    // display the intersection points with the obstacle
                    DrawSomething::DrawPoint("basicsteeringbehavior", intersectionPoints[i], Color(0, 1, 1));
                #endif
                float dist = glm::distance(intersectionPoints[i], TRANSFORM(e)->position);

                if (dist < minDist) {
                    minDist = dist;
                    nearest = intersectionPoints[i];
                    normal = normals[i];
                    obs = obstacle;
                }
            }   
        }
    }
    glm::vec2 force;
    if (minDist != 1000) {
        #if BASIC_STEERING_GRAPHICAL_DEBUG
        // display the real nearest intersection point with any obstacle
        DrawSomething::DrawPoint("basicsteeringbehavior", nearest, Color(0, 0, 0));
        #endif

        // deduce collision normal
        glm::vec2 p (nearest - TRANSFORM(e)->position);

        glm::vec2 tangentSurfaceCollision = glm::vec2(-normal.y, normal.x);
        glm::vec2 lateralForceDirection = glm::sign(glm::dot(p, tangentSurfaceCollision)) * tangentSurfaceCollision;
        glm::vec2 breakingForceDirection = glm::normalize(-p);

        float multiplier = 1.0f + (rectSize.x - minDist) / rectSize.x;
        float latDist = glm::dot(TRANSFORM(e)->position - TRANSFORM(obs)->position, normal);

        latDist = glm::max(0.1f, (rectSize.x - latDist) / rectSize.x);

        #if BASIC_STEERING_GRAPHICAL_DEBUG
        DrawSomething::DrawVec2("basicsteeringbehavior", TRANSFORM(e)->position,
            lateralForceDirection, Color(0, 1, 1));
        DrawSomething::DrawVec2("basicsteeringbehavior", TRANSFORM(e)->position,
            breakingForceDirection, Color(1, 1, 0));
        #endif

        lateralForceDirection *= multiplier * latDist;
        breakingForceDirection *= ((rectSize.x - minDist) / rectSize.x);

        #if BASIC_STEERING_GRAPHICAL_DEBUG
        DrawSomething::DrawVec2("basicsteeringbehavior", TRANSFORM(e)->position,
            lateralForceDirection, Color(0, 0.8, 0.8, 0.5));
        DrawSomething::DrawVec2("basicsteeringbehavior", TRANSFORM(e)->position,
            breakingForceDirection, Color(0.8, 0.8, 0, 0.5));
        #endif

        force = lateralForceDirection + breakingForceDirection;
        
        force = glm::normalize(force) * maxSpeed;
    } else {
        force = glm::vec2(0);
    } 
    #if BASIC_STEERING_GRAPHICAL_DEBUG
    // finally display the final force!
    DrawSomething::DrawVec2("basicsteeringbehavior", TRANSFORM(e)->position, force,
        Color(0, 0, 1, 0.5));
    #endif
    return force;
}

glm::vec2 SteeringBehavior::groupCohesion(Entity e, std::list<Entity>& group, float maxSpeed) {
    if (group.size() == 0) {
        return glm::vec2(0.);
    }

    glm::vec2 averagePosition;

    for (Entity neighbor : group) {
        averagePosition += TRANSFORM(neighbor)->position;
    }
    
    //normalize
    averagePosition /= group.size();

    return seek(e, averagePosition, maxSpeed);
}

glm::vec2 SteeringBehavior::groupAlign(Entity e, std::list<Entity>& group, float maxSpeed) {
    if (group.size() == 0) {
        return glm::vec2(0.);
    }

    glm::vec2 averageDirection;

    for (Entity neighbor : group) {
        averageDirection += glm::rotate(glm::vec2(1, 0), TRANSFORM(neighbor)->rotation);
    }
    
    //normalize
    averageDirection /= group.size();

    if (averageDirection != glm::vec2(0.)) {
        averageDirection = glm::normalize(averageDirection) * maxSpeed;
    }

    auto currentSpeed = PHYSICS(e)->linearVelocity;
    if (currentSpeed != glm::vec2(0.)) {
        currentSpeed = glm::normalize(currentSpeed);
    }
    return averageDirection - currentSpeed;
}

glm::vec2 SteeringBehavior::groupSeparate(Entity e, std::list<Entity>& group, float maxSpeed) {
    glm::vec2 force;

    auto & myPos = TRANSFORM(e)->position;
    for (Entity neighbor : group) {
        auto & neighborPos = TRANSFORM(neighbor)->position;

        auto direction = (myPos - neighborPos);

        // the more neighbor is far away, the less we are attracted by it (norm2)
        force += direction / glm::length2(direction);
    }
    
    if (force != glm::vec2(0.)) {
        force = glm::normalize(force) * maxSpeed;
    }

    return force;
}
