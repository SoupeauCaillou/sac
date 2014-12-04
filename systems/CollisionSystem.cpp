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
    componentSerializer.add(new Property<bool>(HASH("is_a_ray", 0x78a2c1f4), OFFSET(isARay, tc)));

#if SAC_DEBUG
    showDebug = false;
    maximumRayCastPerSec = -1;
    maximumRayCastPerSecAccum = 0;
#endif
}

struct Coll {
    Entity other;
    float t;
    glm::vec2 normal;
};
static void findPotentialCollisions(Entity refEntity, int groupsInside, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, std::vector<Coll>& collisionDuringTheFrame);
static float performRayObjectCollisionInCell(const CollisionComponent* cc, Entity* collidedWithLastFrame, const glm::vec2& origin, const glm::vec2& endA, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, glm::vec2* point);


static bool isInsideCell(const glm::vec2& p, int x, int y, float cellSize, const glm::vec2& worldSize) {
    const glm::vec2& cellCenter = -worldSize * 0.5f + glm::vec2(cellSize * (x+.5f), cellSize *(y+.5f));
    AABB cell;
    cell.left = cellCenter.x - (cellSize * 0.5f);
    cell.right = cell.left + cellSize;
    cell.bottom = cellCenter.y - (cellSize * 0.5f);
    cell.top = cell.bottom + cellSize;
    return IntersectionUtil::pointRectangleAABB(p, cell);
}

#if SAC_DEBUG
static char debugText[4096];
void CollisionSystem::DoUpdate(float dt) {
#else
void CollisionSystem::DoUpdate(float) {
#endif
    #define CELL_SIZE 4.0f
    #define INV_CELL_SIZE (1.0f/CELL_SIZE)

    const int w = glm::floor(worldSize.x * INV_CELL_SIZE);
    const int h = glm::floor(worldSize.y * INV_CELL_SIZE);

#if SAC_DEBUG
    Draw::Clear(HASH("Collision", 0x638cf8ed));
    if (maximumRayCastPerSec > 0)
        maximumRayCastPerSecAccum += maximumRayCastPerSec * dt;

    if (showDebug) {
        if (debug.empty()) {
            for (int j=0; j<h; j++) {
                for (int i=0; i<w; i++) {
                    Entity d = theEntityManager.CreateEntity(HASH("debug_collision_grid", 0x9c1949ab));
                    ADD_COMPONENT(d, Transformation);
                    TRANSFORM(d)->position =
                        -worldSize * 0.5f + glm::vec2(CELL_SIZE * (i+.5f), CELL_SIZE *(j+.5f));
                    TRANSFORM(d)->size = glm::vec2(CELL_SIZE);
                    TRANSFORM(d)->z = 0.95f;
                    ADD_COMPONENT(d, Rendering);
                    RENDERING(d)->color = Color(i%2,j%2,0, 0.1);
                    RENDERING(d)->show = 1;
                    RENDERING(d)->flags = RenderingFlags::NonOpaque;
                    ADD_COMPONENT(d, Text);
                    TEXT(d)->fontName = HASH("typo", 0x5a18f4a9);
                    TEXT(d)->charHeight = CELL_SIZE * 0.2;
                    TEXT(d)->show = 1;
                    TEXT(d)->color.a = 0.3f;
                    TEXT(d)->flags = TextComponent::MultiLineBit;
                    debug.push_back(d);
                }
            }
        }
    } else {
        for (auto d: debug) {
            RENDERING(d)->show = TEXT(d)->show = false;
        }
    }
#endif

    struct Cell {
        Cell() : collidingGroupsInside(0), colliderGroupsInside(0) {}
        // collider/collider collisions forbidden
        std::vector<Entity> collidingEntities;
        std::vector<Entity> colliderEtities;
        std::vector<Entity> rayEntities;
        int collidingGroupsInside, colliderGroupsInside;
    };

    std::vector<Cell> cells;

    cells.reserve(w * h);
    while ((int)cells.size() < w * h) {
        cells.push_back(Cell());
    }

    int minCollidingEntity = INT_MAX, maxCollidingEntity = 0;

    // Assign each entity to cells
    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        if (!cc->isARay && !cc->group)
            continue;
        #if SAC_DEBUG
        if (cc->group & (cc->group - 1)) {
            LOGW("Invalid collision group '" << cc->group << "' for entity " << theEntityManager.entityName(entity) << ". Must be pow2");
        }
        #endif

        cc->collision.count = 0;

        const TransformationComponent* tc = TRANSFORM(entity);

        glm::vec2 pOffset(glm::rotate(tc->size * .5f, tc->rotation));
        glm::vec2 nOffset(-pOffset);

        pOffset += tc->position + worldSize * 0.5f;
        nOffset += tc->position + worldSize * 0.5f;

        int xStart = glm::max(0, glm::min(w-1, (int)glm::floor(pOffset.x * INV_CELL_SIZE)));
        int xEnd = glm::max(0, glm::min(w-1, (int)glm::floor(nOffset.x * INV_CELL_SIZE)));
        if (xStart > xEnd)
            std::swap(xStart, xEnd);

        int yStart = glm::max(0, glm::min(h-1, (int)glm::floor(pOffset.y * INV_CELL_SIZE)));
        int yEnd = glm::max(0, glm::min(h-1, (int)glm::floor(nOffset.y * INV_CELL_SIZE)));
        if (yStart > yEnd)
            std::swap(yStart, yEnd);

        for (int x = xStart; x <= xEnd; x++) {
            for (int y = yStart; y <= yEnd; y++) {
                LOGE_IF(x + y * w >=  w * h, "Incorrect cell index: " << x << '+' << y << '*' << w << " >= " << w << '*' << h);
                Cell& cell = cells[x + y * w];
                if (cc->group > 1 || cc->isARay) {
                    if (cc->isARay) {
                        if (!cc->rayTestDone) {
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

    // ensure array is big enough
    {
        int arrayRequiredSize = MAX_COLLISION_COUNT_PER_ENTITY * (maxCollidingEntity - minCollidingEntity + 1);
        if ((int)collisionEntity.size() < arrayRequiredSize) {
            LOGV(3, "Enlarging collision arrays :" << collisionEntity.size() << " -> " << arrayRequiredSize << '(' << __(minCollidingEntity) << ',' << __(maxCollidingEntity) << ')');
            collisionEntity.resize(arrayRequiredSize);
            collisionPos.resize(arrayRequiredSize);
        }
    }

    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        if (cc->group > 1 || cc->isARay) {
            cc->collision.with = &collisionEntity[MAX_COLLISION_COUNT_PER_ENTITY * (entity - minCollidingEntity)];
            cc->collision.at = &collisionPos[MAX_COLLISION_COUNT_PER_ENTITY * (entity - minCollidingEntity)];
        }
    }

#if SAC_DEBUG
    unsigned debugTextOffset = 0;
#endif
    for (unsigned i=0; i<cells.size(); i++) {
        const Cell& cell = cells[i];

#if SAC_DEBUG
        if (showDebug && debugTextOffset < sizeof(debugTextOffset)) {
            const int x = i % w;
            const int y = i / w;

            int len = 
                snprintf(&debugText[debugTextOffset], sizeof(debugTextOffset),
                    "%d %d\n%lu(%d) %lu(%d)",
                    x, y,
                    cell.collidingEntities.size(), cell.collidingGroupsInside,
                    cell.colliderEtities.size(), cell.colliderGroupsInside);
            TEXT(debug[i])->text = &debugText[debugTextOffset];
            debugTextOffset += len;
        }
#endif

        // Browse colliding entities in this cell
        if (!cell.collidingEntities.empty()) {
            const unsigned count = cell.collidingEntities.size();

            for (unsigned j=0; j<count; j++) {
                const Entity refEntity = cell.collidingEntities[j];

                std::vector<Coll> collisionDuringTheFrame;
                // look for collidingEntities/collidingEntities collisions first
                findPotentialCollisions(refEntity,
                    cell.collidingGroupsInside,
                    cell.collidingEntities.begin() + (j+1),
                    cell.collidingEntities.end(),
                    collisionDuringTheFrame);

                // then look for collidingEntities/colliderEntities collisions
                findPotentialCollisions(refEntity,
                    cell.colliderGroupsInside,
                    cell.colliderEtities.begin(),
                    cell.colliderEtities.end(),
                    collisionDuringTheFrame);

                if (!collisionDuringTheFrame.empty()) {
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

                    for (auto collision: collisionDuringTheFrame) {
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
                        const glm::vec2 s2 = tc2->size * 1.01f;

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
                                collision.t = timing.t1;
                                break;
                            }
                        } while (true);
                    }

                    std::sort(collisionDuringTheFrame.begin(),
                        collisionDuringTheFrame.end(),
                        [] (const Coll& c1, const Coll& c2) -> bool {
                            return c1.t < c2.t;
                        }
                    );

                    int collCount = cc->collision.count = (int) collisionDuringTheFrame.size();

                    for (int i=0; i<collCount; i++) {
                        const Coll& collision = collisionDuringTheFrame[i];

                        cc->collision.with[i] = collision.other;

                        if (cc->restorePositionOnCollision && cc->prevPositionIsValid) {
                            tc->position = glm::lerp(p1[0], p1[1], collision.t);
                            tc->rotation = glm::lerp(r1[0], r1[1], collision.t);
                        }

                        LOGV(2, "Collision: " << theEntityManager.entityName(refEntity) << " -> " << theEntityManager.entityName(collision.other));
                    }
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
                cc->rayTestDone = true;
                cc->collision.count = 0;

                const glm::vec2& origin = TRANSFORM(cell.rayEntities[j])->position;
                const glm::vec2 axis = glm::rotate(glm::vec2(1.0f, 0.0f), TRANSFORM(cell.rayEntities[j])->rotation);
                const glm::vec2 endAxis (origin + axis * glm::max(worldSize.x, worldSize.y));

                glm::vec2 point;

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
                cc->collision.with[0] = 0;

                while(true) {
                    //LOGV (2,  "X=" << X << ", Y=" << Y << '[' << tMaxX << "," << tMaxY << ']');

                    if (cc->collideWith & cell2->collidingGroupsInside) {
                        float nearest1 = performRayObjectCollisionInCell(
                            cc,
                            &cc->collision.with[0],
                            origin,
                            endAxis,
                            cell2->collidingEntities.begin(),
                            cell2->collidingEntities.end(),
                            &cc->collision.at[0]);

                        Entity collidedWithLastFrame2 = 0;
                        glm::vec2 collisionAt2;
                        float nearest2 = performRayObjectCollisionInCell(
                            cc,
                            &collidedWithLastFrame2,
                            origin,
                            endAxis,
                            cell2->colliderEtities.begin(),
                            cell2->colliderEtities.end(),
                            &collisionAt2);

                        if (nearest2 < nearest1) {
                            cc->collision.with[0] = collidedWithLastFrame2;
                            cc->collision.at[0] = collisionAt2;
                        }

                        if (cc->collision.with[0] != 0 && isInsideCell(cc->collision.at[0], X, Y, CELL_SIZE, worldSize)) {
                            cc->collision.count = 1;
                            break;
                        }
                    }

                    // loop
                    if (tMaxX < tMaxY || stepY == 0) {
                        tMaxX += tDeltaX;
                        X += stepX;
                    } else {
                        tMaxY += tDeltaY;
                        Y += stepY;
                    }

                    if (X >= w || X < 0 || Y >= h || Y < 0)
                        break;

                    cell2 = &cells[Y * w + X];
                }

                #if SAC_DEBUG
                if (showDebug) {
                    if (cc->collision.count) {
                        Draw::Vec2(HASH("Collision", 0x638cf8ed), origin, cc->collision.at[0] - origin, Color(1, 0, 0));
                    } else {
                        Draw::Vec2(HASH("Collision", 0x638cf8ed), origin, endAxis - origin, Color(0, 0, 0));
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
}

static float performRayObjectCollisionInCell(
    const CollisionComponent* cc,
    Entity* collidedWithLastFrame,
    const glm::vec2& origin,
    const glm::vec2& endA,
    std::vector<Entity>::const_iterator begin,
    std::vector<Entity>::const_iterator end,
    glm::vec2* point) {

    // Quick exit if this cell doesn't have any entity
    // from our colliding group
    float nearest = FLT_MAX;

    #if SAC_DEBUG
    bool showDebug = theCollisionSystem.showDebug;
    #endif

    for (auto it = begin; it!=end; ++it) {
        const Entity testedEntity = *it;
        if (testedEntity == cc->ignore) continue;

        const CollisionComponent* cc2 = COLLISION(testedEntity);
        if (cc2->group & cc->collideWith) {
            // Test for collision

            glm::vec2 intersectionPoints[2];
            const auto* tc = TRANSFORM(testedEntity);
            int cnt = IntersectionUtil::lineRectangle(origin, endA, tc->position, tc->size, tc->rotation, intersectionPoints);

            for (int i=0; i<cnt; i++) {
                #if SAC_DEBUG
                if (showDebug)
                    Draw::Point(HASH("Collision", 0x638cf8ed), intersectionPoints[i]);
                #endif
                // compute distance2
                float d = glm::distance2(origin, intersectionPoints[i]);
                if (d < nearest) {
                    nearest = d;
                    *point = intersectionPoints[i];
                    *collidedWithLastFrame = testedEntity;
                }
            }
        }
    }

    return nearest;
}

static void findPotentialCollisions(Entity refEntity, int groupsInside, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, std::vector<Coll>& collisionDuringTheFrame) {
    CollisionComponent* cc = COLLISION(refEntity);

    // Quick exit if this cell doesn't have any entity
    // from our colliding group
    if (cc->collideWith & groupsInside) {
        const TransformationComponent* _tc = TRANSFORM(refEntity);
        TransformationComponent tc;
        tc.position = (_tc->position + cc->previousPosition) * 0.5f;
        tc.size = _tc->position - cc->previousPosition + _tc->size;
        tc.rotation = _tc->rotation;// incorrect but...

        for (auto it = begin; it!=end; ++it) {
            const Entity testedEntity = *it;
            if (testedEntity == cc->ignore) continue;

            const CollisionComponent* cc2 = COLLISION(testedEntity);
            if (cc2->group & cc->collideWith) {
                // Test for collision
                if (IntersectionUtil::rectangleRectangle(&tc, TRANSFORM(testedEntity))) {
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
