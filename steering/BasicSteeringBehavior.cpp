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
#include "glm/gtx/projection.hpp"

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

#define CREATE(e, name) \
    e = theEntityManager.CreateEntity(name); \
    ADD_COMPONENT(e, Transformation); \
    ADD_COMPONENT(e, Rendering); \
    RENDERING(e)->opaqueType = RenderingComponent::NON_OPAQUE; \
    RENDERING(e)->show = true;


glm::vec2 SteeringBehavior::avoid(Entity e, const glm::vec2& velocity, std::list<Entity>& obstacles, float maxSpeed) {
#if SAC_DEBUG
    static Entity box = 0, biggerBox, aforce, inter[4];
    if (box == 0) {
        CREATE(box, "avoid_box")
        TRANSFORM(box)->z = .3;
        RENDERING(box)->color = Color(1, 0, 0, .5);
        CREATE(biggerBox, "avoid_biggerBox")
        RENDERING(biggerBox)->color = Color(0, 1, 0, .5);
        TRANSFORM(biggerBox)->z = .4;
        CREATE(aforce, "avoid_force")
        RENDERING(aforce)->color = Color(0, 0, 1, .5);

        for (int i = 0; i < 4; ++i) {
            CREATE(inter[i], "avoid_inter")
            RENDERING(inter[i])->color = Color(1, i / 4.f, i / 4.f, 1);
        }
    }
#endif



    float size = TRANSFORM(e)->size.x * (3 + glm::length(velocity) / maxSpeed);
    const glm::vec2 & rectSize = glm::vec2(size, TRANSFORM(e)->size.y);
    const glm::vec2 & rectPos = TRANSFORM(e)->position + glm::rotate(glm::vec2(rectSize.x * 0.5, 0), TRANSFORM(e)->rotation);
    float rectRot = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(velocity));

    #if SAC_DEBUG
        TRANSFORM(box)->position = rectPos;
        TRANSFORM(box)->size = rectSize;
        TRANSFORM(box)->rotation = rectRot;

        TRANSFORM(biggerBox)->size = glm::vec2(0);

        DrawSomething::DrawPointRestart("avoid");
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


            // we need to get the point of intersection of them to know if its the
            // closer rectangle from entity e

            #if SAC_DEBUG
            DrawSomething::DrawPoint("avoid", TRANSFORM(obstacle)->position);
                TRANSFORM(biggerBox)->position = pos;
                TRANSFORM(biggerBox)->size = TRANSFORM(obstacle)->size + glm::vec2(halfWidth);
                TRANSFORM(biggerBox)->rotation = TRANSFORM(obstacle)->rotation;
                for (int i = 0; i < 4; ++i) TRANSFORM(inter[i])->size = glm::vec2(0.);

            #endif

            int intersectCount = IntersectionUtil::lineRectangle(
                // open-ended line starting at e's position
                TRANSFORM(e)->position, TRANSFORM(e)->position + glm::rotate(glm::vec2(1000, 0), TRANSFORM(e)->rotation),
                // rectangle
                pos, TRANSFORM(obstacle)->size + glm::vec2(halfWidth), TRANSFORM(obstacle)->rotation,
                // result
                intersectionPoints, normals);
            
            for (int i = 0; i < intersectCount; ++i) {
                #if SAC_DEBUG
                    TRANSFORM(inter[i])->position = intersectionPoints[i];
                    TRANSFORM(inter[i])->size = glm::vec2(.1);

                    DrawSomething::DrawPoint("avoid", intersectionPoints[i], Color(0, 1, 1));
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
        #if SAC_DEBUG
        DrawSomething::DrawPoint("avoid", nearest, Color(0, 0, 0));
        #endif

        // deduce collision normal
        glm::vec2 p (nearest - TRANSFORM(e)->position);
        float projNormal = glm::dot(p, normal);

        glm::vec2 lateralForceDirection = normal;//glm::normalize(p - 2 * projNormal * normal);
        glm::vec2 breakingForceDirection = glm::normalize(-p);

        float multiplier = 1.0f + (rectSize.x - minDist) / rectSize.x;
        float latDist = glm::dot(TRANSFORM(e)->position - TRANSFORM(obs)->position, normal);

        force =
            lateralForceDirection * multiplier * latDist + 
            breakingForceDirection * (((rectSize.x - minDist) / rectSize.x) * 0.2f);
        
        force = glm::normalize(force) * maxSpeed;
    } else {
        force = glm::vec2(0);
    } 
    #if SAC_DEBUG
        TRANSFORM(aforce)->position = TRANSFORM(e)->position + force / 2.f;
        TRANSFORM(aforce)->size = glm::vec2(glm::length(force), .1);
        TRANSFORM(aforce)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(force));
    #endif
    return force;
}
