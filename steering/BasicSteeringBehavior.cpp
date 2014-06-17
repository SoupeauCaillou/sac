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
#include "util/Random.h"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

#if SAC_DEBUG
#include "util/Draw.h"
#endif

glm::vec2 SteeringBehavior::seek(Entity e, const glm::vec2& targetPos, float maxSpeed) {
    return seek(TRANSFORM(e)->position, PHYSICS(e)->linearVelocity, targetPos, maxSpeed);
}

glm::vec2 SteeringBehavior::seek(const glm::vec2& pos, const glm::vec2& , const glm::vec2& targetPos, float maxSpeed) {
    glm::vec2 toTarget (targetPos - pos);
    float d = glm::length(toTarget);

    if (d > 0) {
        return toTarget * (maxSpeed / d);
    } else {
        return glm::vec2(.0f);
    }
}

glm::vec2 SteeringBehavior::flee(Entity e, const glm::vec2& targetPos, float maxSpeed) {
    glm::vec2 d (glm::normalize(TRANSFORM(e)->position - targetPos) * maxSpeed);
    return d;
}

glm::vec2 SteeringBehavior::arrive(Entity e, const glm::vec2& targetPos, float maxSpeed, float deceleration) {
    return arrive(TRANSFORM(e)->position, PHYSICS(e)->linearVelocity, targetPos, maxSpeed, deceleration);
}

glm::vec2 SteeringBehavior::arrive(const glm::vec2& pos, const glm::vec2& ,const glm::vec2& targetPos, float maxSpeed, float deceleration) {
    glm::vec2 toTarget (targetPos - pos);
    float d = glm::length(toTarget);

    if (d > 0) {
        toTarget = glm::normalize(toTarget);
        float speed = glm::min(d / deceleration, maxSpeed);
        glm::vec2 desiredVelocity(toTarget * speed);
        return desiredVelocity;
    }
    return glm::vec2(0.0f, 0.0f);
}

glm::vec2 SteeringBehavior::wander(Entity e, WanderParams& params, float maxSpeed) {
    params.target += glm::vec2(
        Random::Float(-1.0f, 1.0f) * params.jitter,
        Random::Float(-1.0f, 1.0f) * params.jitter);
    params.target = glm::normalize(params.target);
    params.target *= params.radius;

    params.debugTarget = TRANSFORM(e)->position + glm::rotate(glm::vec2(params.distance, 0.0f) + params.target, TRANSFORM(e)->rotation);

    return seek(e, params.debugTarget, maxSpeed);
}

#define BASIC_STEERING_GRAPHICAL_DEBUG (1 & SAC_DEBUG)
#if 0
static std::tuple<glm::vec2, glm::vec2> computeOverlappingObstaclesPosSize(Entity refObstacle, const std::list<Entity>& obs) {
    const auto* tc = TRANSFORM(refObstacle);
    glm::vec2 position = tc->position, size = tc->size;
    int count = 1;
    for (Entity o: obs) {
        if (o == refObstacle) continue;
        const auto* tc2 = TRANSFORM(o);
        if (IntersectionUtil::rectangleRectangle(tc, tc2)) {
            #if BASIC_STEERING_GRAPHICAL_DEBUG
            Draw::Rectangle(tc2->position, tc2->size, tc2->rotation, Color(1, 0, 0, 0.5));
            #endif
            position += tc2->position;
            size += tc2->size;
            ++count;
        }
    }
    return std::make_tuple(position / (float)count, size / (float)count);
}
#endif

// TODO: pour calculer la vitesse désirée on peut faire aussi:
// - tourner la vitesse actuelle (pour qu'elle soit tangente à l'obstacle considéré)
// - puis réduire la vitesse tant qu'une collision est détectée (avec n'importe quel obstacle)
glm::vec2 SteeringBehavior::obstacleAvoidance(Entity e, const glm::vec2& velocity, std::list<Entity>& obstacles, float maxSpeed) {
    float size = TRANSFORM(e)->size.x * (1 + 0.5 * glm::length(velocity) / maxSpeed);

    const auto* tc = TRANSFORM(e);
    const glm::vec2 & rectSize = glm::vec2(size, TRANSFORM(e)->size.y);
    const glm::vec2 & rectPos = tc->position + glm::rotate(glm::vec2(rectSize.x * 0.5, 0), tc->rotation);
    float rectRot = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(velocity));

    #if BASIC_STEERING_GRAPHICAL_DEBUG
        // display box-view of the object (where it wants to go)
        Draw::Rectangle(rectPos, rectSize, rectRot, Color(1, 0, 0, .5));
    #endif

    glm::vec2 force;
    glm::vec2 intersectionPoints[4], normals[4];
    const float halfWidth = tc->size.y * 0.5;
    float minDist = 1000;
    Entity obs = 0;
    glm::vec2 nearest, normal;

    for (auto obstacle : obstacles) {
        const auto* tcObstacle = TRANSFORM(obstacle);

        if (IntersectionUtil::rectangleRectangle(tcObstacle->position, tcObstacle->size,
            tcObstacle->rotation, rectPos, rectSize, rectRot)) {

            #if BASIC_STEERING_GRAPHICAL_DEBUG
            // display a box containing the obstacle
            Draw::Rectangle(tcObstacle->position,
                tcObstacle->size + glm::vec2(halfWidth), tcObstacle->rotation,
                Color(0, 1, 0, .5));
            #endif

            float angles[] = {0, glm::radians(25.0f), glm::radians(-25.0f)};

            for (int i=0; i<3; i++) {
                // we need to get the point of intersection of them to know if its the
                // closer rectangle from entity e
                int intersectCount = IntersectionUtil::linePolygon(
                    // open-ended line starting at e's position
                    tc->position, tc->position + glm::rotate(glm::vec2(1000, 0), tc->rotation + angles[i]),
                    // rectangle
                    theTransformationSystem.shapes[tcObstacle->shape],
                    tcObstacle->position, tcObstacle->size /*+ glm::vec2(halfWidth)*/, tcObstacle->rotation,
                    // result
                    intersectionPoints, normals);

                for (int i = 0; i < intersectCount; ++i) {
                    #if BASIC_STEERING_GRAPHICAL_DEBUG
                        // display the intersection points with the obstacle
                        Draw::Point(intersectionPoints[i], Color(0, 1, 1));
                    #endif
                    float dist = glm::distance(intersectionPoints[i], tc->position);

                    // Find the 2 nearest obstacles
                    if (dist < minDist) {
                        minDist= dist;
                        nearest = intersectionPoints[i];
                        normal = normals[i];
                        obs = obstacle;
                    }
                }
            }
        }
    }

    if (obs) {
        #if BASIC_STEERING_GRAPHICAL_DEBUG
        // display the real nearest intersection point with any obstacle
        Draw::Vec2(nearest, normal, Color(0, 0, 0));
        #endif

        // deduce collision normal
        // glm::vec2 p (nearest - tc->position);

        #if 0
        auto groupPosSize = computeOverlappingObstaclesPosSize(obs, obstacles);
        #else
        auto groupPosSize = std::make_tuple(TRANSFORM(obs)->position, TRANSFORM(obs)->size);
        #endif

        // bounding radius
        float bRadius = glm::max(std::get<1>(groupPosSize).x, std::get<1>(groupPosSize).y);

        // local coords of obstacle
        glm::vec2 local =
            glm::vec2(
                glm::dot(std::get<0>(groupPosSize) - tc->position, glm::rotate(glm::vec2(1, 0), tc->rotation)),
                glm::dot(std::get<0>(groupPosSize) - tc->position, glm::rotate(glm::vec2(0, 1), tc->rotation))
            );

        glm::vec2 shift = glm::vec2(-normal.y, normal.x) * glm::sign(local.y) * bRadius;
	#if SAC_DEBUG
	        Draw::Vec2(tc->position, shift, Color(1, 0, 1), "shift");
	#endif

        float l = glm::length(velocity);
        glm::vec2 dv = velocity + shift;
        // brake if necessary
        float brake = glm::max(0.0f, tc->size.x / local.x - 1);

        dv += velocity * (-brake);

        if ((l = glm::length(dv)) > maxSpeed) {
            dv *= maxSpeed / l;
        }

        return dv;
    } else {
        return velocity;
    }

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
    return averageDirection;
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

glm::vec2 SteeringBehavior::boxContainer(Entity e, const glm::vec2& velocity, const glm::vec2& position, const glm::vec2& size, float maxSpeed) {
    const auto * tc = TRANSFORM(e);
    // move in 0.5 sec
    //const auto newPosInHalfSec(tc->position + velocity * 0.5f - position);
    const auto newPosInHalfSec(tc->position + glm::rotate(glm::vec2(tc->size.x, 0.0f), tc->rotation) - position);
    const auto halfSize (size * 0.5f);

    glm::vec2 overShoot(0.0f);
    overShoot.x = glm::max(newPosInHalfSec.x - halfSize.x, -halfSize.x - newPosInHalfSec.x);
    overShoot.y = glm::max(newPosInHalfSec.y - halfSize.y, -halfSize.y - newPosInHalfSec.y);

    if (overShoot.x > 0 || overShoot.y > 0) {
        glm::vec2 direction(0.0f);
        direction.x = (newPosInHalfSec.x > halfSize.x) ? -1 : 1;
        direction.y = (newPosInHalfSec.y > halfSize.y) ? -1 : 1;

        float reactionLength = (0.1 + glm::length(overShoot) * 1.2) * maxSpeed;
        return glm::normalize(velocity) * (maxSpeed - reactionLength) + glm::normalize(direction) * reactionLength;
    } else {
        return velocity;
    }
}

glm::vec2 SteeringBehavior::wallAvoidance(Entity e, const glm::vec2& velocity,
    const std::list<Entity>& walls, float maxSpeed) {

    // Use 3 probes (0°, 45°, -45°) and in case of hits add a force along
    // the normal of the wall proportionnal to the hit
    constexpr float feelers[3*2] = {
        0.0f, 1.0f,
        0.69f, 0.85,
        -0.69f, 0.85
    };

    float closestIP = 0;
    Entity closestWall = 0;
    float overShoot = 0;
    glm::vec2 wallNormal;

    const auto* tc = TRANSFORM(e);
    const auto& myPos = tc->position;

    const glm::vec2 feelerStart = myPos + glm::rotate(glm::vec2(tc->size.x * 0.25f, 0.0f), tc->rotation);

    glm::vec2 feelerEnd[3];

    for (int i=0; i<3; i++) {
        feelerEnd[i] = feelerStart + feelers[2*i + 1] * glm::rotate(glm::vec2(tc->size.x * feelers[2*i+1], 0.0f), tc->rotation + feelers[2*i]);

        #if SAC_DEBUG
            Draw::Vec2(
                feelerStart,
                feelerEnd[i] - feelerStart,
                Color(0, 1, 0, 0.75));
        #endif
    }

    for (auto & wall : walls) {
        const auto* tc2 = TRANSFORM(wall);
        LOGT_IF(tc2->rotation != 0, "Wall must be aligned");
        #if 0
        glm::vec2 dir, wallA, wallB;
        if (tc2->size.x < tc2->size.y) {
            dir = glm::rotate(glm::vec2(0.0f, tc2->size.y * 0.5f), tc2->rotation);
            wallA = tc2->position - dir;
            wallB = tc2->position + dir;
        } else {
            dir = glm::rotate(glm::vec2(tc2->size.x * 0.5f, 0.0f), tc2->rotation);
            wallA = tc2->position + dir;
            wallB = tc2->position - dir;
        }
        #else
        glm::vec2 wallA(tc2->position), wallB(tc2->position);
        if (tc2->size.x < tc2->size.y) {
            wallA.y -= tc2->size.y;
            wallB.y += tc2->size.y;
        } else {
            wallA.x += tc2->size.x;
            wallB.x -= tc2->size.x;
        }
        #endif

        if (!IntersectionUtil::lineCircle(wallA, wallB, tc->position, tc->size.x * 2)) {
            continue;
        }

        for (int i=0; i<3; i++) {
            glm::vec2 intersectionPoint;

            if (IntersectionUtil::lineLine(
                    feelerStart,
                    feelerEnd[i],
                    wallA,
                    wallB,
                    &intersectionPoint)) {
                #if SAC_DEBUG
                Draw::Vec2(
                    feelerStart,
                    feelerEnd[i] - feelerStart,
                    Color(1, 0, 0, 1));
                Draw::Vec2(
                    wallA,
                    wallB,
                    Color(0, 0, 1));
                #endif

                auto dist = glm::distance2(feelerStart, intersectionPoint);

                if (closestWall == 0 || dist < closestIP) {
                    closestIP = dist;
                    closestWall = wall;
                    overShoot = glm::distance(feelerEnd[i], intersectionPoint) / (tc->size.x * feelers[2*i+1]); /* glm::distance(feelerEnd[i], feelerStart); */
                    auto w = wallA - wallB;
                    wallNormal = glm::normalize(glm::vec2(-w.y, w.x));
                    // orient wall normal toward vehicle
                    wallNormal *= glm::sign(glm::dot(myPos - intersectionPoint, wallNormal));
                }
            }
        }
    }

    if (closestWall) {
        float reactionLength = (0.1 + overShoot * 1.2) * maxSpeed;
        return glm::normalize(velocity) * (maxSpeed - reactionLength) + wallNormal * reactionLength;
    } else {
        return velocity;
    }
}
