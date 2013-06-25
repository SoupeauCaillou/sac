/*
    This file is part of sac.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    RecursiveRunner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    RecursiveRunner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
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

INSTANCE_IMPL(CollisionSystem);

CollisionSystem::CollisionSystem() : ComponentSystemImpl<CollisionComponent>("Collision") {
    CollisionComponent tc;
    componentSerializer.add(new Property<int>("group", OFFSET(group, tc), 0));
    componentSerializer.add(new Property<int>("collide_with", OFFSET(collideWith, tc), 0));
}

struct Coll {
    Entity other;
    float t;
    glm::vec2 normal;
};
static void findPotentialCollisions(Entity refEntity, int groupsInside, std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end, std::vector<Coll>& collisionDuringTheFrame);

void CollisionSystem::DoUpdate(float) {
    #define CELL_SIZE 4.0f
    #define INV_CELL_SIZE (1.0f/CELL_SIZE)

    const int w = glm::floor(worldSize.x * INV_CELL_SIZE);
    const int h = glm::floor(worldSize.y * INV_CELL_SIZE);

#if SAC_DEBUG
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
                float c = ((i+j) % 2) ? 0.1:0.3;
                RENDERING(d)->color = Color(c, c, c, 0.3);
                RENDERING(d)->show = 1;
                ADD_COMPONENT(d, Text);
                TEXT(d)->fontName = "typo";
                TEXT(d)->charHeight = CELL_SIZE * 0.2;
                TEXT(d)->show = 1;
                debug.push_back(d);
            }
        }

    }
#endif

    struct Cell {
        Cell() : collidingGroupsInside(0), colliderGroupsInside(0) {}
        // collider/collider collisions forbidden
        std::vector<Entity> collidingEntities;
        std::vector<Entity> colliderEtities;
        int collidingGroupsInside, colliderGroupsInside;
    };

    std::vector<Cell> cells;

    cells.resize(w * h);

    // Assign each entity to cells
    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        if (!cc->group)
            continue;

        const TransformationComponent* tc = TRANSFORM(entity);

        glm::vec2 pOffset(glm::rotate(tc->size * .5f, tc->rotation));
        glm::vec2 nOffset(-pOffset);

        pOffset += tc->position + worldSize * 0.5f;
        nOffset += tc->position + worldSize * 0.5f;

        int xStart = glm::floor(pOffset.x * INV_CELL_SIZE);
        int xEnd = glm::floor(nOffset.x * INV_CELL_SIZE);
        if (xStart > xEnd)
            std::swap(xStart, xEnd);

        int yStart = glm::floor(pOffset.y * INV_CELL_SIZE);
        int yEnd = glm::floor(nOffset.y * INV_CELL_SIZE);
        if (yStart > yEnd)
            std::swap(yStart, yEnd);

        for (int x = xStart; x <= xEnd; x++) {
            for (int y = yStart; y <= yEnd; y++) {
                Cell& cell = cells[x + y * w];
                if (cc->group > 1) {
                    cell.collidingEntities.push_back(entity);
                    cell.collidingGroupsInside |= cc->group;
                } else {
                    cell.colliderEtities.push_back(entity);
                    cell.colliderGroupsInside |= cc->group;
                }
            }
        }
    }

    for (unsigned i=0; i<cells.size(); i++) {
        const Cell& cell = cells[i];

        #if SAC_DEBUG
        std::stringstream ss;
        ss << cell.collidingEntities.size() << '('
            << cell.collidingGroupsInside << "), "
            << cell.colliderEtities.size() << '('
            << cell.colliderGroupsInside << ')';
        TEXT(debug[i])->text = ss.str();
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

                    float minT = 1.0;
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
                        collision.t = 0.5;
                        float step = 0.25;
                        int iteration = 10;
                        do {
                            glm::vec2 _pos1 = glm::lerp(p1[0], p1[1], collision.t);
                            float _r1 = glm::lerp(r1[0], r1[1], collision.t);
                            glm::vec2 _pos2 = glm::lerp(p2[0], p2[1], collision.t);
                            float _r2 = glm::lerp(r2[0], r2[1], collision.t);

                            if (IntersectionUtil::rectangleRectangle(
                                _pos1, s1, _r1,
                                _pos2, s2, _r2)) {
                                collision.t -= step;
                            } else {
                                collision.t += step;
                                if (collision.t > minT)
                                    break;
                            }
                            step *= 0.5;

                            if (--iteration == 0) {
                                /*
                                LOGI(t);
                                TRANSFORM(refEntity)->position = _pos1;
                                TRANSFORM(refEntity)->rotation = _r1;
                                TRANSFORM(collision.other)->position = _pos2;
                                TRANSFORM(collision.other)->rotation = _r2;
                                */
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

                    TRANSFORM(refEntity)->position = glm::lerp(p1[0], p1[1], collision.t);
                    TRANSFORM(refEntity)->rotation = glm::lerp(r1[0], r1[1], collision.t);
                    TRANSFORM(collision.other)->position = glm::lerp(p2[0], p2[1], collision.t);
                    TRANSFORM(collision.other)->rotation = glm::lerp(r2[0], r2[1], collision.t);
                }
            }
        }
        #if SAC_DEBUG
        else if (cell.colliderEtities.empty()) {
            TEXT(debug[i])->text.clear();
        }
        #endif
    }

    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        cc->previousPosition = TRANSFORM(entity)->position;
        cc->previousRotation = TRANSFORM(entity)->rotation;
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
                    collisionDuringTheFrame.push_back(c);


                    LOGI("Collision : " <<
                        theEntityManager.entityName(refEntity) << " <-> " <<
                        theEntityManager.entityName(testedEntity));
                }
            }
        }
    }
}
