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



#include "CollisionSystem.h"
#include "TransformationSystem.h"
#if SAC_DEBUG
#include "RenderingSystem.h"
#include "TextSystem.h"
#endif
#include "util/IntersectionUtil.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/norm.hpp>

#include "util/DrawSomething.h"
INSTANCE_IMPL(CollisionSystem);

CollisionSystem::CollisionSystem() : ComponentSystemImpl<CollisionComponent>("Collision") {
    CollisionComponent tc;
    componentSerializer.add(new Property<int>("group", OFFSET(group, tc), 0));
    componentSerializer.add(new Property<int>("collide_with", OFFSET(collideWith, tc), 0));
    componentSerializer.add(new Property<bool>("restore_position_on_collision", OFFSET(restorePositionOnCollision, tc)));
    componentSerializer.add(new Property<bool>("is_a_ray", OFFSET(isARay, tc)));

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
static void performRayObjectCollisionInCell(CollisionComponent* cc, int groupsInside, const glm::vec2& origin, const glm::vec2& endA, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, float& nearest, glm::vec2& point);

void CollisionSystem::DoUpdate(float dt) {
    #define CELL_SIZE 4.0f
    #define INV_CELL_SIZE (1.0f/CELL_SIZE)

    const int w = glm::floor(worldSize.x * INV_CELL_SIZE);
    const int h = glm::floor(worldSize.y * INV_CELL_SIZE);

#if SAC_DEBUG
    if (maximumRayCastPerSec > 0)
        maximumRayCastPerSecAccum += maximumRayCastPerSec * dt;

    if (showDebug) {
        if (debug.empty()) {
            for (int j=0; j<h; j++) {
                for (int i=0; i<w; i++) {
                    Entity d = theEntityManager.CreateEntity("debug_collision_grid");
                    ADD_COMPONENT(d, Transformation);
                    TRANSFORM(d)->position =
                        -worldSize * 0.5f + glm::vec2(CELL_SIZE * (i+.5f), CELL_SIZE *(j+.5f));
                    TRANSFORM(d)->size = glm::vec2(CELL_SIZE);
                    TRANSFORM(d)->z = 0.95;
                    ADD_COMPONENT(d, Rendering);
                    RENDERING(d)->color = Color(i%2,j%2,0, 0.1);
                    RENDERING(d)->show = 1;
                    RENDERING(d)->opaqueType = RenderingComponent::NON_OPAQUE;
                    ADD_COMPONENT(d, Text);
                    TEXT(d)->fontName = "typo";
                    TEXT(d)->charHeight = CELL_SIZE * 0.2;
                    TEXT(d)->show = 1;
                    TEXT(d)->color.a = 0.3;
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

    // Assign each entity to cells
    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        if (!cc->isARay && !cc->group)
            continue;
        cc->collidedWithLastFrame = 0;

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
                } else {
                    cell.colliderEtities.push_back(entity);
                    cell.colliderGroupsInside |= cc->group;
                }
            }
        }
    END_FOR_EACH()

    // DrawSomething::Clear();

    for (unsigned i=0; i<cells.size(); i++) {
        const Cell& cell = cells[i];

#if SAC_DEBUG
        if (showDebug) {
            const int x = i % w;
            const int y = i / w;

            std::stringstream ss;
            ss << x << ' ' << y << '\n'
                << cell.collidingEntities.size() << '('
                << cell.collidingGroupsInside << "), "
                << cell.colliderEtities.size() << '('
                << cell.colliderGroupsInside << ')';
            TEXT(debug[i])->text = ss.str();
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
                    const glm::vec2 p1[] = {
                        COLLISION(refEntity)->previousPosition,
                        TRANSFORM(refEntity)->position
                    };
                    const float r1[] = {
                        COLLISION(refEntity)->previousRotation,
                        TRANSFORM(refEntity)->rotation
                    };
                    const glm::vec2 s1 = TRANSFORM(refEntity)->size * 1.01f;

                    for (auto collision: collisionDuringTheFrame) {
                        const glm::vec2 p2[] = {
                            COLLISION(collision.other)->previousPosition,
                            TRANSFORM(collision.other)->position
                        };
                        const float r2[2] = {
                            COLLISION(collision.other)->previousRotation,
                            TRANSFORM(collision.other)->rotation
                        };
                        const glm::vec2 s2 = TRANSFORM(collision.other)->size * 1.01f;

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

                    collisionDuringTheFrame.resize(1);

                    const Coll& collision = collisionDuringTheFrame[0];
                    const glm::vec2 p2[] = {
                        COLLISION(collision.other)->previousPosition,
                        TRANSFORM(collision.other)->position
                    };
                    const float r2[2] = {
                        COLLISION(collision.other)->previousRotation,
                        TRANSFORM(collision.other)->rotation
                    };

                    if (COLLISION(refEntity)->restorePositionOnCollision) {
                        TRANSFORM(refEntity)->position = glm::lerp(p1[0], p1[1], collision.t);
                        TRANSFORM(refEntity)->rotation = glm::lerp(r1[0], r1[1], collision.t);
                    }
                    COLLISION(refEntity)->collidedWithLastFrame = collision.other;

                    if (COLLISION(collision.other)->restorePositionOnCollision) {
                        TRANSFORM(collision.other)->position = glm::lerp(p2[0], p2[1], collision.t);
                        TRANSFORM(collision.other)->rotation = glm::lerp(r2[0], r2[1], collision.t);
                    }
                    COLLISION(collision.other)->collidedWithLastFrame = refEntity;
                    LOGV(2, "Collision: " << theEntityManager.entityName(refEntity) << " -> " << theEntityManager.entityName(collision.other));
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
                cc->collidedWithLastFrame = 0;
                const glm::vec2& origin = TRANSFORM(cell.rayEntities[j])->position;
                const glm::vec2 axis = glm::rotate(glm::vec2(1.0f, 0.0f), TRANSFORM(cell.rayEntities[j])->rotation);
                const glm::vec2 endAxis (origin + axis * glm::max(worldSize.x, worldSize.y));
                cc->collisionAt = endAxis;
                float nearest = glm::distance2(endAxis, origin);
                glm::vec2 point;

                // from http://www.cse.yorku.ca/~amana/research/grid.pdf
                const int stepX = glm::sign(axis.x);
                const int stepY = glm::sign(axis.y);

                const float dX = (CELL_SIZE * (xStart + ((stepX > 0) ? 1 : 0)) - (origin.x + worldSize.x * 0.5));
                float tMaxX = dX / axis.x;
                const float dY = (CELL_SIZE * (yStart + ((stepY > 0) ? 1 : 0)) - (origin.y + worldSize.y * 0.5));
                float tMaxY = dY / axis.y;
                const float tDeltaX = (CELL_SIZE / axis.x) * stepX;
                const float tDeltaY = (CELL_SIZE / axis.y) * stepY;

                LOGV(2, origin << " / " << xStart << ", " << yStart << "/" << stepX << "," << stepY << '/' << axis << '/' << dX << "->" << tMaxX << ", " << dY << "->" << tMaxY);
                int X = xStart;
                int Y = yStart;

                const Cell* cell2 = &cell;
                while(true) {
                    LOGV(2, "X=" << X << ", Y=" << Y << '[' << tMaxX << "," << tMaxY << ']');

                    performRayObjectCollisionInCell(
                        cc,
                        cell2->collidingGroupsInside,
                        origin,
                        endAxis,
                        cell2->collidingEntities.begin(),
                        cell2->collidingEntities.end(),
                        nearest,
                        cc->collisionAt);

                    performRayObjectCollisionInCell(
                        cc,
                        cell2->colliderGroupsInside,
                        origin,
                        endAxis,
                        cell2->colliderEtities.begin(),
                        cell2->colliderEtities.end(),
                        nearest,
                        cc->collisionAt);

                    if (cc->collidedWithLastFrame != 0)
                        break;


                    // loop
                    if (tMaxX < tMaxY) {
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
            }
        }
        #if SAC_DEBUG
        else if (cell.colliderEtities.empty()) {
            // TEXT(debug[i])->text.clear();
        }
        #endif
    }

    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        cc->previousPosition = TRANSFORM(entity)->position;
        cc->previousRotation = TRANSFORM(entity)->rotation;
    END_FOR_EACH()
}

static void performRayObjectCollisionInCell(CollisionComponent* cc, int groupsInside, const glm::vec2& origin, const glm::vec2& endA, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, float& nearest, glm::vec2& point) {
    // Quick exit if this cell doesn't have any entity
    // from our colliding group
    if (cc->collideWith & groupsInside) {
        for (auto it = begin; it!=end; ++it) {
            const Entity testedEntity = *it;
            const CollisionComponent* cc2 = COLLISION(testedEntity);
            if (cc2->group & cc->collideWith) {
                // Test for collision

                glm::vec2 intersectionPoints[2];
                const auto* tc = TRANSFORM(testedEntity);
                int cnt = IntersectionUtil::lineRectangle(origin, endA, tc->position, tc->size, tc->rotation, intersectionPoints);

                for (int i=0; i<cnt; i++) {
                    // DrawSomething::DrawPoint("collision", intersectionPoints[i]);
                    // compute distance2
                    float d = glm::distance2(origin, intersectionPoints[i]);
                    if (d < nearest) {
                        nearest = d;
                        point = intersectionPoints[i];
                        cc->collidedWithLastFrame = testedEntity;
                    }
                }
            }
        }
    }
}

static void findPotentialCollisions(Entity refEntity, int groupsInside, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, std::vector<Coll>& collisionDuringTheFrame) {
    CollisionComponent* cc = COLLISION(refEntity);

    // Quick exit if this cell doesn't have any entity
    // from our colliding group
    if (cc->collideWith & groupsInside) {
        const TransformationComponent* tc = TRANSFORM(refEntity);

        for (auto it = begin; it!=end; ++it) {
            const Entity testedEntity = *it;
            const CollisionComponent* cc2 = COLLISION(testedEntity);
            if (cc2->group & cc->collideWith) {
                // Test for collision
                if (IntersectionUtil::rectangleRectangle(tc, TRANSFORM(testedEntity))) {
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
