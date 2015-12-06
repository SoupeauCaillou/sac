#include <UnitTest++.h>

#include <glm/glm.hpp>
#include "systems/SpatialPartitionSystem.h"
#include "systems/TransformationSystem.h"
#include "util/Random.h"

struct PosSizeRot {
    PosSizeRot(glm::vec2 p = glm::vec2(0.0f)) : pos(p), size(1.0f), rotation(0.0f) {}
    PosSizeRot(glm::vec2 p, glm::vec2 s) : pos(p), size(s), rotation(0.0f) {}
    PosSizeRot(glm::vec2 p, glm::vec2 s, float r) : pos(p), size(s), rotation(r) {}
    glm::vec2 pos, size;
    float rotation;
};

void createEntities(PosSizeRot* ps, int count) {
    for (int i=0; i<count; i++) {
        Entity e = i + 1;
        theTransformationSystem.Add(e);
        theSpatialPartitionSystem.Add(e);

        TRANSFORM(e)->position = ps->pos;
        TRANSFORM(e)->size = ps->size;
        TRANSFORM(e)->rotation = ps->rotation;
        ps++;
    }
}

struct SPSTestSetup {
    SPSTestSetup() {
        TransformationSystem::CreateInstance();
        SpatialPartitionSystem::CreateInstance();
    }
    ~SPSTestSetup() {
        TransformationSystem::DestroyInstance();
        SpatialPartitionSystem::DestroyInstance();
    }
};

TEST_FIXTURE(SPSTestSetup, SingleCenteredEntity)
{
    Entity e[] = { 1 };
    PosSizeRot psz[] = {
        PosSizeRot(glm::vec2(0, 0))
    };
    createEntities(psz, 1);

    theSpatialPartitionSystem.DoUpdate(0);

    CHECK_EQUAL(0u, SPATIAL_PARTITION(e[0])->partitionId);
}

TEST_FIXTURE(SPSTestSetup, ThreeEntities)
{
    Entity e[] = { 1, 2, 3 };
    PosSizeRot psz[] = {
        PosSizeRot(glm::vec2(-5, 5)),
        PosSizeRot(glm::vec2(5, 5)),
        PosSizeRot(glm::vec2(-5, -5))
    };
    createEntities(psz, 3);

    theSpatialPartitionSystem.DoUpdate(0);

    for (int i=0; i<3; i++) {
        //LOGI(SPATIAL_PARTITION(e[i])->partitionId);
        CHECK(SPATIAL_PARTITION(e[i])->partitionId != SPATIAL_PARTITION(e[(i+1) % 3])->partitionId);
    }
}

TEST_FIXTURE(SPSTestSetup, NodeSplit)
{
    // MAX_OBJECT_PER_NODE 10
    Entity e[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    PosSizeRot psz[11];
    for (int i=0; i<11; i++) {
        psz[i] = PosSizeRot(glm::vec2(Random::Float(1, 20)));
    }
    createEntities(psz, 11);
    theSpatialPartitionSystem.Delete(e[10]);

    theSpatialPartitionSystem.DoUpdate(0);
    // they should all live in the same
    for (int i=0; i<10; i++) {
        LOGI(__(i) << ':' << SPATIAL_PARTITION(e[i])->partitionId << ' ' << __(psz[i].pos));
        CHECK(SPATIAL_PARTITION(e[i])->partitionId == SPATIAL_PARTITION(e[(i+1) % 10])->partitionId);
    }

    // add an extra-one to force a split
    theSpatialPartitionSystem.Add(e[10]);

    theSpatialPartitionSystem.DoUpdate(0);

    bool hasSplitted = false;
    for (int i=0; i<11 && !hasSplitted; i++) {
        LOGI(SPATIAL_PARTITION(e[i])->partitionId);
        hasSplitted = SPATIAL_PARTITION(e[i])->partitionId != SPATIAL_PARTITION(e[(i+1) % 11])->partitionId;
    }
    // At least one of them should have been splitted
    CHECK(hasSplitted);
}
