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

#include "SpatialPartitionSystem.h"
#include "util/SerializerProperty.h"
#include "util/IntersectionUtil.h"
#include "TransformationSystem.h"

#if SAC_DEBUG
#include "util/Draw.h"
#endif

/* Build a simple quadtree */
#define MAX_OBJECT_PER_NODE 10
#define MAX_DEPTH 16
struct QuadTree {
    QuadTree() {
        memset(entities, 0, sizeof(entities));
        parent = 0;
        memset(children, 0, sizeof(children));
    }
    Entity entities[MAX_OBJECT_PER_NODE];
    uint32_t parent;
    uint32_t children[4];
    int size;
};

std::vector<QuadTree> tree;

static float halfSizes[MAX_DEPTH];

INSTANCE_IMPL(SpatialPartitionSystem);

SpatialPartitionSystem::SpatialPartitionSystem() : ComponentSystemImpl<SpatialPartitionComponent>(HASH("SpatialPartition", 0x35df9814)) {
    SpatialPartitionComponent tc;
    componentSerializer.add(new Property<int>(HASH("mode", 0xbfaa2be1), OFFSET(mode, tc)));
    componentSerializer.add(new Property<int>(HASH("partition_id", 0x3d899463), OFFSET(partitionId, tc)));

    #if SAC_DEBUG
    showDebug = false;
    #endif

    halfSizes[0] = 10000.0f * 0.5f;

    for (int i=1; i<MAX_DEPTH; i++) {
        halfSizes[i] = halfSizes[0] / glm::pow(2.0f, (float)i);
    }
}

static void updateEntity(Entity e, SpatialPartitionComponent* spc);

#if SAC_DEBUG
void drawDebug(int index, const glm::vec2& position, int depth = 0) {
    const QuadTree& node = tree[index];
    // top-left, top-right, bottom-left, bottom-right
    const glm::vec2 offset[] = {
        glm::vec2(-0.5f, 0.5f), glm::vec2(0.5f), glm::vec2(-0.5f), glm::vec2(0.5f, -0.5f)
    };

    LOGI(__(position) << '/' << (halfSizes[node.size]*2) << ' ' << __(depth));
    bool hasNoChild = true;
    for (int i=0; i<4; i++) {
        if (node.children[i]) {
            drawDebug(
                node.children[i],
                position + glm::vec2(halfSizes[node.size]) * offset[i],
                depth +1);
            hasNoChild = false;
        }
    }

    if (hasNoChild) {
        Draw::Rectangle(position, glm::vec2(halfSizes[node.size] * 2), 0.0f, Color::palette(index / (float)tree.size(), 0.9));
    } else {
        LOGI("NO " << __(position) << '/' << glm::vec2(halfSizes[node.size] * 2));
    }
}
#endif

void SpatialPartitionSystem::DoUpdate(float) {
    QuadTree root;
    root.size = 0;
    tree.clear();
    tree.push_back(root);

    FOR_EACH_ENTITY_COMPONENT(SpatialPartition, e, comp)
        updateEntity(e, comp);
    }

    #if SAC_DEBUG
    if (showDebug) {
        Draw::Point(glm::vec2(0.0f));
        LOGI("debug");
        drawDebug(0, glm::vec2(0.0f));
    }
    #endif
}


static uint32_t computeNode(const glm::vec2* points, int node);
static void computePoints(Entity e, glm::vec2* out);
static uint32_t insertEntity(Entity e);
static int splitNode(uint32_t node);
static void insertEntityInNode(Entity e, uint32_t node);

static int nextFreeSpotInNode(const QuadTree& node) {
    int i = 0;
    do {
        if (node.entities[i] == 0) {
            break;
        }
    } while ((++i) < MAX_OBJECT_PER_NODE);

    return i;
}

static void updateEntity(Entity e, SpatialPartitionComponent* spc) {
    uint32_t current = spc->partitionId;
    bool found = false;
    /*for (int i=0; i<MAX_OBJECT_PER_NODE && !found; i++) {
        if (tree[current].entities[i] == e) {
            found = true;
            tree[current].entities[i] = 0;

            for (int j=MAX_OBJECT_PER_NODE - 1; j>=0; j--) {
                if (tree[current].entities[j]) {
                    tree[current].entities[i] = tree[current].entities[j];
                    tree[current].entities[j] = 0;
                    break;
                }
            }
        }
    }*/
    insertEntity(e);
}

static uint32_t insertEntity(Entity e) {
    glm::vec2 points[4];

    computePoints(e, points);
    uint32_t node = computeNode(points, 0);

    insertEntityInNode(e, node);
    return node;
}

static void insertEntityInNode(Entity e, uint32_t node) {
    int freeSpot = nextFreeSpotInNode(tree[node]);

    if (freeSpot < MAX_OBJECT_PER_NODE) {
    } else {
        //LOGV(2, "Split node");
        freeSpot = splitNode(node);
    }

    if (freeSpot == -1) {
        LOGI("Meh unassigned");
        return;
    }
    //LOGV(2, "insert " << e << " in " << node << " at spot " << freeSpot);
    tree[node].entities[freeSpot] = e;

    SPATIAL_PARTITION(e)->partitionId = node;
}

static void computePoints(Entity e, glm::vec2* out) {
    AABB aabb;
    IntersectionUtil::computeAABB(TRANSFORM(e), aabb);
    out[0] = glm::vec2(aabb.left, aabb.top);
    out[1] = glm::vec2(aabb.right, aabb.top);
    out[2] = glm::vec2(aabb.right, aabb.bottom);
    out[3] = glm::vec2(aabb.left, aabb.bottom);
}

static int splitNode(uint32_t node)
    {glm::vec2 points[4];

    for (int i=0; i<MAX_OBJECT_PER_NODE; i++) {
        Entity e = tree[node].entities[i];
        computePoints(e, points);
        // /LOGI("splitNode: " << e);
        uint32_t newNode = computeNode(points, node);
        if (newNode != node) {
            insertEntityInNode(e, newNode);
            return i;
        }
    }
    return -1;
}

static uint32_t computeNode(const glm::vec2* points, int node) {
    const float childHalfSize = halfSizes[node] * 0.5f;

    // top-left, top-right, bottom-left, bottom-right
    const float leftCoeff[] =   { -1,  0, -1,  0 };
    const float rightCoeff[] =   {  0,  1,  0,  1 };
    const float topCoeff[] =    {  1,  1,  0,  0 };
    const float bottomCoeff[] = {  0,  0,  -1, -1 };

    // if the 4 points belongs to a node, insert
    for (int i=0; i<4; i++) {
        AABB aabb;
        aabb.left = leftCoeff[i] * childHalfSize;
        aabb.right = rightCoeff[i] * childHalfSize;
        aabb.top = topCoeff[i] * childHalfSize;
        aabb.bottom = bottomCoeff[i] * childHalfSize;

        // if all points are inside this node
        bool inside = true;
        for (int j=0; j<4 && inside; j++) {
            inside &= IntersectionUtil::pointRectangleAABB(points[j], aabb);
        }
        if (inside) {
            //LOGI("ASSIGN");
            QuadTree& currentNode = tree[node];
            if (currentNode.children[i] == 0) {
                currentNode.children[i] = tree.size();

                QuadTree child;
                child.size = tree[node].size + 1;
                LOGF_IF(child.size == MAX_DEPTH, "oops MAX_DEPTH reached, need plan B");
                child.parent = node;
                tree.push_back(child);
            }
            return currentNode.children[i];
        }
    }
    //LOGI(points[0] <<' ' << points[1] << ' ' << points[2] << ' ' << points[3]);

    // if it cannot fit in a child, assign to current node
    return node;
}
