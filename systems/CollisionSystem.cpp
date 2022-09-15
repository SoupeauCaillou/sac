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


#if !DISABLE_COLLISION_SYSTEM
#include "CollisionSystem.h"
#include "TransformationSystem.h"
#if SAC_DEBUG
#include "RenderingSystem.h"
#include "TextSystem.h"
#endif
#include "util/IntersectionUtil.h"
#include <glm/gtx/norm.hpp>
#include <algorithm>

#include "BackInTimeSystem.h"
#include "SpatialPartitionSystem.h"
#include "base/EntityManager.h"
#include "util/Draw.h"
#include "util/SerializerProperty.h"
#define SAC_TWEAK_IMPLEMENTATION
#include "util/Tweak.h"

INSTANCE_IMPL(CollisionSystem);

#define MAX_COLLISION_COUNT_PER_ENTITY 4

CollisionSystem::CollisionSystem() : ComponentSystemImpl<CollisionComponent>(HASH("Collision", 0x638cf8ed)) {
    CollisionComponent tc;
    componentSerializer.add(new Property<int>(HASH("group", 0xbf3bf34d), OFFSET(group, tc), 0));
    componentSerializer.add(new Property<int>(HASH("collide_with", 0x6b658240), OFFSET(collideWith, tc), 0));
    componentSerializer.add(new Property<bool>(HASH("restore_transformation_at_collision", 0x34ec9de3), OFFSET(restoreTransformation.atCollision, tc)));
    componentSerializer.add(new Property<bool>(HASH("restore_transformation_before_collision", 0x9d4fb7b7), OFFSET(restoreTransformation.beforeCollision, tc)));
    // componentSerializer.add(new Property<bool>(HASH("is_a_ray", 0x78a2c1f4), OFFSET(ray.is, tc)));

#if SAC_DEBUG
    showDebug = false;
    maximumRayCastPerSec = -1;
    maximumRayCastPerSecAccum = 0;
#endif
}

struct Coll {
    Entity other;
    // real collision time is > t.t1 and < t.t2
    Interval<float> t;
    glm::vec2 normal;
};

struct EntityData {
    Entity e;
    int group;
    int collideWith;
    AABB aabb;
};

struct Cell {
    Cell() : collidingGroups(0) {}

    std::vector<EntityData> entities;

    int collidingGroups;

    // std::vector<Entity> rayEntities;
    // int X, Y;
};

struct TransformInterpolation {
    glm::vec2 position[2];
    glm::vec2 size[2];
    float rotation[2];
    bool moving, scaling, rotating;

    TransformInterpolation(const TransformationComponent* tc, const BackInTimeComponent* spc) {
        position[1] = tc->position; position[0] = spc->position;
        moving = position[1] != position[0];
        size[1] = tc->size; size[0] = spc->size;
        scaling = size[1] != size[0];
        rotation[1] = tc->rotation; rotation[0] = spc->rotation;
        rotating = rotation[1] != rotation[0];
    }

    glm::vec2 pos(float t) { return moving ? glm::lerp(position[0], position[1], t) : position[1]; }
    glm::vec2 s(float t) { return scaling ? glm::lerp(size[0], size[1], t) : size[1]; }
    float rot(float t) { return rotating ? glm::lerp(rotation[0], rotation[1], t) : rotation[1]; }
};

static bool determineCollisionTimestamp(EntityData e1, EntityData e2, float* timeBefore, float* timeAt) {
    TransformInterpolation ti1(TRANSFORM(e1.e), BACK_IN_TIME(e1.e));
    TransformInterpolation ti2(TRANSFORM(e2.e), BACK_IN_TIME(e2.e));

    float tNope = 0.0f;
    float tYes = FLT_MAX;

    TWEAK(int, collision1PassIterationSteps) = 10;
    for (int step = 0; step < collision1PassIterationSteps; step++) {
        float t = step / (float)collision1PassIterationSteps;
        bool collision =
            IntersectionUtil::rectangleRectangle(
                ti1.pos(t), ti1.s(t), ti1.rot(t),
                ti2.pos(t), ti2.s(t), ti2.rot(t));
        if (collision) {
            tYes = t;
            break;
        } else {
            tNope = t;
        }
    }

    if (tYes == tNope) {
        *timeBefore = *timeAt = 0;
        return true;
    }
    if (tYes == FLT_MAX) {
        return false;
    }

    // refine tYes
    TWEAK(int, collision2ndPassIterationSteps) = 10;
    int step = 0;
    for (step=0; step<collision2ndPassIterationSteps && (tYes - tNope) > 0.01; step++) {
        float t = (tNope + tYes) * 0.5f;

        if (IntersectionUtil::rectangleRectangle(
            ti1.pos(t), ti1.s(t), ti1.rot(t),
            ti2.pos(t), ti2.s(t), ti2.rot(t))) {
            tYes = t;
        } else {
            tNope = t;
        }
    }
    *timeBefore = tNope;
    *timeAt = tYes;
    return true;
}

static void insertCollisionResult(CollisionComponent* cc, Entity e, float ts) {
    for (int i=0; i<cc->collision.count; i++) {
        if (ts < cc->collision.at[i]) {
            // insert
            int toMove =
                glm::min(cc->collision.count, MAX_COLLISION_COUNT_PER_ENTITY - 1) -
                i;

            if (toMove) {
                memmove(
                    &cc->collision.with[i+1],
                    &cc->collision.with[i],
                    sizeof(Entity) * toMove);
                memmove(
                    &cc->collision.at[i+1],
                    &cc->collision.at[i],
                    sizeof(float) * toMove);
            }
            cc->collision.with[i] = e;
            cc->collision.at[i] = ts;
            return;
        }
    }
    if (cc->collision.count < MAX_COLLISION_COUNT_PER_ENTITY) {
        cc->collision.with[cc->collision.count] = e;
        cc->collision.at[cc->collision.count] = ts;
        cc->collision.count++;
    }
}

void CollisionSystem::DoUpdate(float dt) {

    // int minCollidingEntity = INT_MAX, maxCollidingEntity = 0;
    std::vector<Cell> cells;
    int gridPitch = theSpatialPartitionSystem.gridSize.x;
    cells.resize(gridPitch * theSpatialPartitionSystem.gridSize.y);
    int collidingEntitiesCount = 0;

    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        cc->collision.count = 0;

        // ignore disabled (group == 0) entities
        if (cc->group == 0) {
            continue;
        }

        collidingEntitiesCount += (cc->collideWith > 0);

        const auto* sp = SPATIAL_PARTITION(entity);

        if (sp->count == 0) {
            continue;
        }
        const auto* spCells = theSpatialPartitionSystem.getCells(sp->cellOffset);
        for (int i=0; i<sp->count; i++) {
            glm::ivec2 coords = spCells[i];
            Cell& cell = cells[gridPitch * coords.y + coords.x];
            const auto* hc = BACK_IN_TIME(entity);
            EntityData d;
            d.e = entity;
            AABB nowBefore[2];
            IntersectionUtil::computeAABB(
                TRANSFORM(entity),
                nowBefore[0]);
            IntersectionUtil::computeAABB(
                hc->position,
                hc->size,
                hc->rotation,
                nowBefore[1]);
            d.aabb = IntersectionUtil::mergeAABB(nowBefore, 2);
            d.group = cc->group;
            d.collideWith = cc->collideWith;
            cell.entities.push_back(d);
            if (cc->collideWith > 0) {
                cell.collidingGroups |= cc->group;
            }
        }
    END_FOR_EACH()

    if (collidingEntitiesCount == 0) {
        LOGT_EVERY_N(60, "...");
        return;
    }
    // ensure results array is big enough
    {
        int arrayRequiredSize = MAX_COLLISION_COUNT_PER_ENTITY * collidingEntitiesCount;
        if ((int)collisionEntity.size() < arrayRequiredSize) {
            LOGV(3, "Enlarging collision arrays :" << collisionEntity.size() << " -> " << arrayRequiredSize);
            collisionEntity.resize(arrayRequiredSize);
            collisionTimestamp.resize(arrayRequiredSize);
            collisionNormal.resize(arrayRequiredSize);
        }
    }

    // assign results locations
    {
        int index = 0;
        FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
            if (cc->collideWith > 0 /*|| cc->ray.is*/) {
                cc->collision.with = &collisionEntity[MAX_COLLISION_COUNT_PER_ENTITY * index];
                cc->collision.at = &collisionTimestamp[MAX_COLLISION_COUNT_PER_ENTITY * index];
                cc->collision.normal = &collisionNormal[MAX_COLLISION_COUNT_PER_ENTITY * index];
                index++;
            }
        }
    }

    for (int i=0; i<(int)cells.size(); i++) {
        const Cell& cell = cells[i];

        // Browse entities in this cell
        if (!cell.entities.empty()) {
            const int count = (int)cell.entities.size();

            for (int j=0; j<count; j++) {
                const EntityData& reference = cell.entities[j];

                if (reference.collideWith == 0) {
                    continue;
                }

                for (int k=0; k<count; k++) {
                    if (k == j) {
                        continue;
                    }

                    const EntityData& test = cell.entities[k];
                    bool checkNeeded =
                        (reference.collideWith & test.group) |
                        (test.collideWith & reference.group);

                    if (!checkNeeded) {
                        continue;
                    }
                    if (IntersectionUtil::rectangleRectangleAABB(
                        reference.aabb,
                        test.aabb)) {
                        #if SAC_DEBUG
                        if (showDebug) {
                            Color color = Color::random(0.8f);
                            Draw::RectangleAABB(reference.aabb, color);
                            Draw::RectangleAABB(test.aabb, color);
                        }
                        #endif

                        // determine the time of collision
                        float tBefore, tAt;

                        if (!determineCollisionTimestamp(reference, test, &tBefore, &tAt)) {
                            continue;
                        }

                        {
                            auto* cc = COLLISION(reference.e);
                            if (reference.collideWith & test.group) {
                                insertCollisionResult(cc, test.e, tAt);
                            }
                            if (cc->restoreTransformation.atCollision ||
                                cc->restoreTransformation.beforeCollision) {
                                float ts = cc->restoreTransformation.atCollision ?
                                    tAt : tBefore;
                                auto* tc = TRANSFORM(reference.e);
                                auto* bc = BACK_IN_TIME(reference.e);
                                tc->position = glm::lerp(bc->position, tc->position, ts);
                                tc->size = glm::lerp(bc->size, tc->size, ts);
                                tc->rotation = glm::lerp(bc->rotation, tc->rotation, ts);
                            }
                        }

                        {
                            auto* cc = COLLISION(test.e);
                            if (test.collideWith & reference.group) {
                                insertCollisionResult(cc, reference.e, tAt);
                            }
                            if (cc->restoreTransformation.atCollision ||
                                cc->restoreTransformation.beforeCollision) {
                                float ts = cc->restoreTransformation.atCollision ?
                                    tAt : tBefore;
                                auto* tc = TRANSFORM(test.e);
                                auto* bc = BACK_IN_TIME(test.e);
                                tc->position = glm::lerp(bc->position, tc->position, ts);
                                tc->size = glm::lerp(bc->size, tc->size, ts);
                                tc->rotation = glm::lerp(bc->rotation, tc->rotation, ts);
                            }
                        }
                    }
                }
            }
        }
    }
#if 0

                std::vector<Coll> potentialcollisionDuringTheFrame;




                // look for collidingEntities/collidingEntities collisions first
                findPotentialCollisions(refEntity,
                    cell.collidingGroupsInside,
                    cell.collidingEntities.begin() + (j+1),
                    cell.collidingEntities.end(),
                    potentialcollisionDuringTheFrame);

                // then look for collidingEntities/colliderEntities collisions
                findPotentialCollisions(refEntity,
                    cell.colliderGroupsInside,
                    cell.colliderEtities.begin(),
                    cell.colliderEtities.end(),
                    potentialcollisionDuringTheFrame);

                if (!potentialcollisionDuringTheFrame.empty()) {
                    auto* cc = COLLISION(refEntity);
                    auto* tc = TRANSFORM(refEntity);

                    const glm::vec2 p1[] = {
                        cc->previousPosition,
                        tc->position
                    };
                    const float r1[] = {
                        cc->previousRotation,
                        tc->rotation
                    };
                    const glm::vec2 s1 = tc->size * 1.01f;

                    for (auto collision: potentialcollisionDuringTheFrame) {
                        const auto* cc2 = COLLISION(collision.other);
                        const auto* tc2 = TRANSFORM(collision.other);
                        const glm::vec2 p2[] = {
                            cc2->previousPosition,
                            tc2->position
                        };
                        const float r2[2] = {
                            cc2->previousRotation,
                            tc2->rotation
                        };
                        const glm::vec2 s2 = tc2->size;//    * 1.01f;

                        // resolve collision, and keep only 1
                        Interval<float> timing (0, 1);
                        int iteration = 5;
                        do {
                            const float t = timing.lerp(0.5);
                            glm::vec2 _pos1 = glm::lerp(p1[0], p1[1], t);
                            float _r1 = glm::lerp(r1[0], r1[1], t);
                            glm::vec2 _pos2 = glm::lerp(p2[0], p2[1], t);
                            float _r2 = glm::lerp(r2[0], r2[1], t);

                            if (IntersectionUtil::rectangleRectangle(
                                _pos1, s1, _r1,
                                _pos2, s2, _r2)) {
                                timing.t2 = t;
                            } else {
                                timing.t1 = t;
                            }

                            if (--iteration == 0) {
                                collision.t = timing;
                                break;
                            }
                        } while (true);
                    }

                    // remove
                    std::remove_if(potentialcollisionDuringTheFrame.begin(),
                        potentialcollisionDuringTheFrame.end(),
                        [] (const Coll& c) -> bool {
                            return c.t.t2 <= c.t.t1;
                        }
                    );

                    std::sort(potentialcollisionDuringTheFrame.begin(),
                        potentialcollisionDuringTheFrame.end(),
                        [] (const Coll& c1, const Coll& c2) -> bool {
                            return c1.t.t1 < c2.t.t1;
                        }
                    );

                    bool firstCollisionFound = false;
                    int collCount = 0;
                    Interval<float> firstCollisionTime;

                    glm::vec2 at[4], normals[4];
                    for (unsigned i=0; i<potentialcollisionDuringTheFrame.size() && collCount < 4; i++) {
                        const Coll& collision = potentialcollisionDuringTheFrame[i];

                        if (firstCollisionFound) {
                            if (collision.t.t1 > firstCollisionTime.t1) {
                                break;
                            }
                        }

                        int valid = IntersectionUtil::rectangleRectangle(
                            tc, // should use position @ collision.time.t2
                            TRANSFORM(collision.other),
                            at,
                            normals);

                        if (!valid) {
                            continue;
                        }

                        if (!firstCollisionFound && cc->restorePositionOnCollision && cc->prevPositionIsValid) {
                            firstCollisionTime = collision.t;
                            tc->position = glm::lerp(p1[0], p1[1], firstCollisionTime.t1);
                            tc->rotation = glm::lerp(r1[0], r1[1], firstCollisionTime.t1);
                            firstCollisionFound = true;
                        }

                        for (int j=0; j<valid && collCount < 4; j++) {
                            cc->collision.with[collCount] = collision.other;
                            cc->collision.at[collCount] = at[j];
                            cc->collision.normal[collCount] = normals[j];
                            collCount++;
                        }

#if SAC_DEBUG
                        if (showDebug) {
                            Draw::Vec2(cc->collision.at[i],
                                cc->collision.normal[i],
                                Color(0,0,0));
                        }
#endif
                        LOGV(2, "Collision: " << theEntityManager.entityName(refEntity) << " -> " << theEntityManager.entityName(collision.other));
                    }

                    cc->collision.count = collCount;

                }
            }
        }

        if (!cell.rayEntities.empty()) {
            const unsigned count = cell.rayEntities.size();
            const int xStart = i % w;
            const int yStart = i / w;

            for (unsigned j=0; j<count; j++) {
                auto* cc = COLLISION(cell.rayEntities[j]);
#if SAC_DEBUG
                if (maximumRayCastPerSec > 0) {
                    if (maximumRayCastPerSecAccum < 1)
                        break;
                    else
                        maximumRayCastPerSecAccum--;
                }
#endif
                cc->ray.testDone = true;
                cc->collision.count = 0;

                const glm::vec2& origin = TRANSFORM(cell.rayEntities[j])->position;
                const glm::vec2 axis = glm::rotate(glm::vec2(1.0f, 0.0f), TRANSFORM(cell.rayEntities[j])->rotation);
                const glm::vec2 endAxis (origin + axis * glm::max(worldSize.x, worldSize.y));

                // from http://www.cse.yorku.ca/~amana/research/grid.pdf
                const int stepX = glm::sign(axis.x);
                const int stepY = glm::sign(axis.y);

                const float dX = (CELL_SIZE * (xStart + ((stepX > 0) ? 1 : 0)) - (origin.x + worldSize.x * 0.5f));
                float tMaxX = dX / axis.x;
                const float dY = (CELL_SIZE * (yStart + ((stepY > 0) ? 1 : 0)) - (origin.y + worldSize.y * 0.5f));
                float tMaxY = dY / axis.y;

                const float tDeltaX = (CELL_SIZE / axis.x) * stepX;
                const float tDeltaY = (CELL_SIZE / axis.y) * stepY;

                LOGV(2, origin << " / " << xStart << ", " << yStart << "/" << stepX << "," << stepY << '/' << axis << '/' << dX << "->" << tMaxX << ", " << dY << "->" << tMaxY);
                int X = xStart;
                int Y = yStart;

                const Cell* cell2 = &cell;

                int collisionCount = 0;

                while(true) {
                    //LOGV (2,  "X=" << X << ", Y=" << Y << '[' << tMaxX << "," << tMaxY << ']');

                    if (cc->collideWith & cell2->collidingGroupsInside) {
                        collisionCount = performRayObjectCollisionInCell(
                            cc,
                            origin,
                            endAxis,
                            *cell2,
                            Category::Colliding,
                            cc->collision.with,
                            cc->collision.at,
                            collisionCount,
                            cc->ray.maxCollision
                            );
                    }

                    if (cc->collideWith & cell2->colliderGroupsInside) {
                        collisionCount = performRayObjectCollisionInCell(
                            cc,
                            origin,
                            endAxis,
                            *cell2,
                            Category::Collider,
                            cc->collision.with,
                            cc->collision.at,
                            collisionCount,
                            cc->ray.maxCollision);
                    }

                    cc->collision.count = collisionCount;

                    if (collisionCount >= cc->ray.maxCollision) {
                        cc->collision.count = glm::min(cc->ray.maxCollision, cc->collision.count);
                        break;
                    }

                    // loop
                    if (tMaxX < tMaxY || stepY == 0) {
                        tMaxX += tDeltaX;
                        X += stepX;
                    } else {
                        tMaxY += tDeltaY;
                        Y += stepY;
                    }

                    if (X >= w || X < 0 || Y >= h || Y < 0) {
                        break;
                    }

                    cell2 = &cells[Y * w + X];
                }

                #if SAC_DEBUG
                if (showDebug) {
                    Color colors[] = {
                        Color(1, 0, 0, 0.3), Color(1, 0, 0, 0.3),
                        Color(0, 1, 0, 0.3), Color(0.5, 0.5, 0, 0.3),
                        Color(0, 0, 1, 0.3), Color(0, 0.5, 0.5, 0.3),
                    };
                    const glm::vec2* prev = &origin;
                    Entity prevEntity = 0;
                    for (int i=0; i<cc->collision.count; i++) {
                        Draw::Vec2(HASH("Collision", 0x638cf8ed), (*prev), cc->collision.at[i] - (*prev),
                            colors[(2 * i + (prevEntity == cc->collision.with[i]) ) % 6 ]);
                        // Draw::Point(HASH("Collision", 0x638cf8ed), cc->collision.at[i], Color(0.2, 0.2, 0.2, 0.6));
                        prev = &cc->collision.at[i];
                        prevEntity = cc->collision.with[i];

                    }
                }
                #endif
            }
        }
    }
    #endif
}

#if 0
static int performRayObjectCollisionInCell(
    const CollisionComponent* cc,
    const glm::vec2& origin,
    const glm::vec2& endA,
    const Cell& cell,
    Category::Enum tested,
    Entity* withs,
    glm::vec2* points,
    int collisionCount,
    int maxColl) {

    LOGF_IF(maxColl >= MAX_COLLISION_COUNT_PER_ENTITY, "Raycast intersection count limited to 4 (" << maxColl << " requested");

    float nearest[MAX_COLLISION_COUNT_PER_ENTITY] = {
        FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX
    };
    for (int i=0; i<collisionCount; i++) {
        nearest[i] = glm::distance2(origin, points[i]);
    }

    const glm::vec2& worldSize = theCollisionSystem.worldSize;

    std::vector<Entity>::const_iterator begin, end;
    if (tested == Category::Colliding) {
        begin = cell.collidingEntities.begin();
        end = cell.collidingEntities.end();
    } else {
        begin = cell.colliderEtities.begin();
        end = cell.colliderEtities.end();
    }

    for (auto it = begin; it!=end; ++it) {
        const Entity testedEntity = *it;
        if (testedEntity == cc->ignore) continue;

        const CollisionComponent* cc2 = COLLISION(testedEntity);
        if (cc2->group & cc->collideWith) {
            // Test for collision

            glm::vec2 intersectionPoints[4];
            const auto* tc = TRANSFORM(testedEntity);
            int cnt = IntersectionUtil::lineRectangle(origin, endA, tc->position, tc->size, tc->rotation, intersectionPoints);

            for (int i=0; i<cnt; i++) {
                /* only valid if inside current cell */
                if (!isInsideCell(intersectionPoints[i], cell.X, cell.Y, CELL_SIZE, worldSize)) {
                    continue;
                }

                // compute distance2
                float d = glm::distance2(origin, intersectionPoints[i]);

                int index = collisionCount;
                for (int j=0; j<collisionCount; j++) {
                    /* ignore duplicate */
                    if (glm::abs(d - nearest[j]) < 0.001 && withs[j] == testedEntity) {
                        index = MAX_COLLISION_COUNT_PER_ENTITY;
                        break;
                    }
                    if (d < nearest[j]) {
                        for (int k=glm::min(collisionCount, MAX_COLLISION_COUNT_PER_ENTITY - 1); k>j; k--) {
                            nearest[k] = nearest[k - 1];
                            points[k] = points[k - 1];
                            withs[k] = withs[k - 1];
                        }
                        index = j;
                        break;
                    }
                }

                if (index < MAX_COLLISION_COUNT_PER_ENTITY) {
                    nearest[index] = d;
                    points[index] = intersectionPoints[i];
                    withs[index] = testedEntity;
                    collisionCount = glm::min(collisionCount + 1, MAX_COLLISION_COUNT_PER_ENTITY);
                }
            }
        }
    }

    return collisionCount;
}

static void findPotentialCollisions(Entity refEntity, int groupsInside, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, std::vector<Coll>& collisionDuringTheFrame) {
    CollisionComponent* cc = COLLISION(refEntity);

    // Quick exit if this cell doesn't have any entity
    // from our colliding group
    if (cc->collideWith & groupsInside) {
        const TransformationComponent* _tc = TRANSFORM(refEntity);

        AABB aabb[2];
        IntersectionUtil::computeAABB(_tc, aabb[0]);
        IntersectionUtil::computeAABB(cc->previousPosition, _tc->size, cc->previousRotation, aabb[1]);

        AABB merge = IntersectionUtil::mergeAABB(aabb, 2);

        for (auto it = begin; it!=end; ++it) {
            const Entity testedEntity = *it;
            if (testedEntity == cc->ignore) continue;

            const CollisionComponent* cc2 = COLLISION(testedEntity);
            if (cc2->group & cc->collideWith) {
                AABB other;
                IntersectionUtil::computeAABB(TRANSFORM(testedEntity), other, true);

                // Test for collision
                if (IntersectionUtil::rectangleRectangleAABB(merge, other)) {
                    // try to find the exact collision time
                    Coll c;
                    c.other = testedEntity;
                    c.t = 0;
                    collisionDuringTheFrame.push_back(c);


                    LOGV(2, "Collision : " <<
                        theEntityManager.entityName(refEntity) << " <-> " <<
                        theEntityManager.entityName(testedEntity));
                }
            }
        }
    }
}

glm::vec2 CollisionSystem::collisionPointToNormal(const glm::vec2& point, const TransformationComponent* tc) {
    glm::vec2 diffNorm = glm::rotate(point - tc->position, -tc->rotation) / tc->size;
    glm::vec2 normal;

    if (glm::abs(diffNorm.x) >= glm::abs(diffNorm.y)) {
        normal = glm::vec2(glm::sign(diffNorm.x), 0.0f);
    } else {
        normal = glm::vec2(0.0f, glm::sign(diffNorm.y));
    }
    return glm::rotate(normal, tc->rotation);
}
#endif

#endif
