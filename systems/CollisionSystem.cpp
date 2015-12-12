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

#include "util/Draw.h"
#include "util/SerializerProperty.h"
#include "base/EntityManager.h"

INSTANCE_IMPL(CollisionSystem);

#define MAX_COLLISION_COUNT_PER_ENTITY 4

CollisionSystem::CollisionSystem() : ComponentSystemImpl<CollisionComponent>(HASH("Collision", 0x638cf8ed)) {
    CollisionComponent tc;
    componentSerializer.add(new Property<int>(HASH("group", 0xbf3bf34d), OFFSET(group, tc), 0));
    componentSerializer.add(new Property<int>(HASH("collide_with", 0x6b658240), OFFSET(collideWith, tc), 0));
    componentSerializer.add(new Property<bool>(HASH("restore_position_on_collision", 0x9c45df9f), OFFSET(restorePositionOnCollision, tc)));
    // componentSerializer.add(new Property<bool>(HASH("is_a_ray", 0x78a2c1f4), OFFSET(ray.is, tc)));
    componentSerializer.add(new Property<bool>(HASH("is_static", 0x104445ee), OFFSET(isStatic, tc)));

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

struct Cell {
    Cell() : collidingGroupsInside(0), colliderGroupsInside(0) {}
    // collider/collider collisions forbidden
    std::vector<Entity> collidingEntities;
    std::vector<Entity> colliderEtities;
    std::vector<Entity> rayEntities;
    int collidingGroupsInside, colliderGroupsInside;
    int X, Y;
};

static void findPotentialCollisions(Entity refEntity, int groupsInside, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, std::vector<Coll>& collisionDuringTheFrame);

namespace Category
{
    enum Enum {
        Colliding,
        Collider,
    };
}
static int performRayObjectCollisionInCell(
    const CollisionComponent* cc,
    const glm::vec2& origin,
    const glm::vec2& endA,
    const Cell& cell,
    Category::Enum tested,
    Entity* withs,
    glm::vec2* points,
    int collisionCount,
    int maxColl);


static bool isInsideCell(const glm::vec2& p, int x, int y, float cellSize, const glm::vec2& worldSize) {
    const glm::vec2& cellCenter = -worldSize * 0.5f + glm::vec2(cellSize * (x+.5f), cellSize *(y+.5f));
    AABB cell;
    cell.left = cellCenter.x - (cellSize * 0.5f);
    cell.right = cell.left + cellSize;
    cell.bottom = cellCenter.y - (cellSize * 0.5f);
    cell.top = cell.bottom + cellSize;
    return IntersectionUtil::pointRectangleAABB(p, cell);
}

static void updateAABBWithPreviousFrame(const TransformationComponent* tc, const CollisionComponent* cc, AABB& aabb) {
    AABB frames[2];
    frames[1] = aabb;
    IntersectionUtil::computeAABB(
        cc->previousPosition, tc->size, cc->previousRotation, frames[0]);
    aabb = IntersectionUtil::mergeAABB(frames, 2);
}

#if SAC_DEBUG
static char debugText[8096];
void CollisionSystem::DoUpdate(float dt) {
#else
void CollisionSystem::DoUpdate(float) {
#endif
    #if 1
    LOGT_EVERY_N(600, "finish me");
    #else

    int minCollidingEntity = INT_MAX, maxCollidingEntity = 0;

    // Assign each entity to cells
    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        if (!cc->ray.is && !cc->group)
            continue;

        #if SAC_DEBUG
        if (cc->group & (cc->group - 1)) {
            LOGW("Invalid collision group '" << cc->group << "' for entity " << theEntityManager.entityName(entity) << ". Must be pow2");
        }
        LOGF_IF(theSpatialPartitionSystem.Get(entity, false) == NULL,
            "Entity with Collision component must also have SpatialPartition component");
        #endif

        cc->collision.count = 0;

        const TransformationComponent* tc = TRANSFORM(entity);
        if (cc->ray.is) {
            tc->size = glm::vec2(0.0f);
        }

        for (int x = xStart; x <= xEnd; x++) {
            for (int y = yStart; y <= yEnd; y++) {

                if (!cc->isStatic || cc->ray.is) {
                    if (cc->ray.is) {
                        if (!cc->ray.testDone) {
                            cell.rayEntities.push_back(entity);
                        }
                    } else {
                        cell.collidingEntities.push_back(entity);
                        cell.collidingGroupsInside |= cc->group;
                    }
                    maxCollidingEntity = glm::max(maxCollidingEntity, (int)entity);
                    minCollidingEntity = glm::min(minCollidingEntity, (int)entity);
                } else {
                    cell.colliderEtities.push_back(entity);
                    cell.colliderGroupsInside |= cc->group;
                }
            }
        }
    END_FOR_EACH()

    for (size_t i=0; i<cells.size(); i++) {
        const Cell& cell = cells[i];

        size_t entityCount = cell.size();
        for (size_t j=0; j<entityCount; j++) {
            Entity e1 = cell[j];
            CollisionComponent* cc1 = COLLISION(e1);

            // skip static entities
            if (cc1->isStatic) {
                continue;
            }

            const TransformationComponent* tc1 = TRANSFORM(e1);
            // build aabb covering previous position and current position
            AABB aabb1;
            IntersectionUtil::computeAABB(tc1, aabb1);
            updateAABBWithPreviousFrame(tc1, cc1, aabb1);

            // collide with all others
            for (size_t k=0; k<entityCount; k++) {
                Entity e2 = cell[k];
                CollisionComponent* cc2 = COLLISION(e2);

                // can't collide with each other
                if (!(cc1->collideWith & cc2->group)) {
                    continue;
                }

                const TransformationComponent* tc2 = TRANSFORM(e2);
                AABB aabb2;
                IntersectionUtil::computeAABB(tc2, aabb2);
                if (!cc2->isStatic) {
                    updateAABBWithPreviousFrame(tc2, cc2, aabb2);
                }

                if (IntersectionUtil::rectangleRectangleAABB(aabb1, aabb2)) {
                    // potential collision during the frame
                }
            }
        }
    }

    // ensure array is big enough
    {
        int arrayRequiredSize = MAX_COLLISION_COUNT_PER_ENTITY * (maxCollidingEntity - minCollidingEntity + 1);
        if ((int)collisionEntity.size() < arrayRequiredSize) {
            LOGV(3, "Enlarging collision arrays :" << collisionEntity.size() << " -> " << arrayRequiredSize << '(' << __(minCollidingEntity) << ',' << __(maxCollidingEntity) << ')');
            collisionEntity.resize(arrayRequiredSize);
            collisionPos.resize(arrayRequiredSize);
            collisionNormal.resize(arrayRequiredSize);
        }
    }

    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        if (cc->group > 1 || cc->ray.is) {
            cc->collision.with = &collisionEntity[MAX_COLLISION_COUNT_PER_ENTITY * (entity - minCollidingEntity)];
            cc->collision.at = &collisionPos[MAX_COLLISION_COUNT_PER_ENTITY * (entity - minCollidingEntity)];
            cc->collision.normal = &collisionNormal[MAX_COLLISION_COUNT_PER_ENTITY * (entity - minCollidingEntity)];
        }
    }

#if SAC_DEBUG
    unsigned debugTextOffset = 0;
#endif
    for (unsigned i=0; i<cells.size(); i++) {
        const Cell& cell = cells[i];

#if SAC_DEBUG
        if (showDebug && debugTextOffset < sizeof(debugText)) {
            const int x = i % w;
            const int y = i / w;

            int len =
                snprintf(&debugText[debugTextOffset], sizeof(debugText),
                    "%d %d\n%lu(%d) %lu(%d)",
                    x, y,
                    cell.collidingEntities.size(), cell.collidingGroupsInside,
                    cell.colliderEtities.size(), cell.colliderGroupsInside);
            TEXT(debug[i])->text = &debugText[debugTextOffset];
            debugTextOffset += len;
            TEXT(debug[i])->show = true;
        }
#endif

        // Browse colliding entities in this cell
        if (!cell.collidingEntities.empty()) {
            const unsigned count = cell.collidingEntities.size();

            for (unsigned j=0; j<count; j++) {
                const Entity refEntity = cell.collidingEntities[j];

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

    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        cc->previousPosition = TRANSFORM(entity)->position;
        cc->previousRotation = TRANSFORM(entity)->rotation;
        cc->prevPositionIsValid = true;
    END_FOR_EACH()
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
