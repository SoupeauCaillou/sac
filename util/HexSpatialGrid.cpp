#if !DISABLE_GRID_SYSTEM

#include "HexSpatialGrid.h"

#include "base/Entity.h"
#include "base/EntityManager.h"
#include "base/Log.h"

#include "systems/GridSystem.h"
#include "systems/opengl/Polygon.h"
#include "systems/TransformationSystem.h"

#include "util/IntersectionUtil.h"

#include <list>
#include <map>
#include <utility>

HexSpatialGrid::~HexSpatialGrid(){
}

HexSpatialGrid::HexSpatialGrid(int pW, int pH, float hexagonWidth) : w(pW), h(pH) {
    LOGF_IF((h % 2) == 0, "Must use odd height");
    LOGF_IF((w % 2) == 0, "Must use odd width");

    float hexaHeight = hexagonWidth / (glm::sqrt(3.0f) * 0.5f);
    size = hexaHeight * 0.5f;

    const float vertSpacing = (3.0f/4) * hexaHeight;
    const float horiSpacing = hexagonWidth;

    GridPos endCell, firstCell = positionSizeToGridPos(
        glm::vec2(horiSpacing * (int)(-w*0.5f), vertSpacing * (int)(-h*0.5f)));

    // varies on z (r) first
    int qStart = firstCell.q;
    for (int z=0; z < h; z++) {
        // then, compute q
        for (int q=0; q<w; q++) {
            endCell = GridPos(qStart + q, firstCell.r + z);
            cells.insert(std::make_pair(endCell, Cell()));
        }
        if ((z % 2) == 1) {
            qStart--;
        }
    }
    LOGE_IF((endCell.q != -firstCell.q) || (endCell.r != -firstCell.r), "Incoherent first/last cell");
}

bool HexSpatialGrid::isPosValid(const GridPos& pos) const {
    return cells.find(pos) != cells.end();
}

bool HexSpatialGrid::isPathBlockedAt(const GridPos& npos, Entity* by) const {
    LOGF_IF(!isPosValid(npos), "Invalid pos used: " << npos);
    for (const auto& e: (cells.find(npos)->second).entities) {
        if (theGridSystem.Get(e, false) && GRID(e)->blocksPath) {
            if (by) *by = e;
            return true;
        }
    }
    return false;
}

bool HexSpatialGrid::isVisibilityBlockedAt(const GridPos& npos) const {
    // don't use this function because the result may depend on the point of view due to houses behaviour
    // (if we are already in a house, we might see neighbours house blocks, otherwise this should return false for example)
    LOGF_IF(!isPosValid(npos), "Invalid pos used: " << npos);
    for (const auto& e: (cells.find(npos)->second).entities) {
        if (theGridSystem.Get(e, false) && GRID(e)->blocksVision) {
            return true;
        }
    }
    return false;
}

int HexSpatialGrid::gridPosMoveCost(const GridPos&, const GridPos& to) const {
    for (const auto& e: (cells.find(to)->second).entities) {
        auto* gc = theGridSystem.Get(e, false);
        if (gc && gc->moveCost > 0) {
            return gc->moveCost;
        }
    }
    // default cost is 1
    return 1;
}

glm::vec2 HexSpatialGrid::gridPosToPosition(const GridPos& gp) const {
    return glm::vec2(
        size * glm::sqrt(3.0f) * (gp.q + gp.r * 0.5),
        size * 3.0f * 0.5 * gp.r);
}

GridPos HexSpatialGrid::positionToGridPos(const glm::vec2& pos) const {
    return positionSizeToGridPos(pos);
}

std::vector<GridPos> HexSpatialGrid::getNeighbors(const GridPos& pos, bool enableInvalidPos) const {
    std::vector<GridPos> n;
    int offsets[] = {
        1, 0,
        1, -1,
        0, -1,
        -1, 0,
        -1, 1,
        0, 1
    };
    for (int i=0; i<6; i++) {
        GridPos p(pos.q + offsets[2 * i], pos.r +offsets[2 * i + 1]);
        if (enableInvalidPos || isPosValid(p)) {
            n.push_back(p);
        }
    }
    return n;
}

unsigned HexSpatialGrid::computeGridDistance(const glm::vec2& p1, const glm::vec2& p2) const {
    return computeGridDistance(positionToGridPos(p1), positionToGridPos(p2));
}

unsigned HexSpatialGrid::computeGridDistance(const GridPos& p1, const GridPos& p2) const {
    return (glm::abs(p1.q - p2.q) + glm::abs(p1.r - p2.r)
            + glm::abs(p1.r + p1.q - p2.q - p2.r)) / 2;
}

float HexSpatialGrid::computeRealDistance(const glm::vec2& p1, const glm::vec2& p2) const {
    return computeRealDistance(positionToGridPos(p1), positionToGridPos(p2));
}

float HexSpatialGrid::computeRealDistance(const GridPos& p1, const GridPos& p2) const {
    //could be improved
    return (glm::distance(gridPosToPosition(p1), gridPosToPosition(p2)));
}

void HexSpatialGrid::forEachCellDo(std::function<void(const GridPos&)> fnct) {
    for (auto& a: cells) {
        fnct(a.first);
    }
}

void HexSpatialGrid::addEntityAt(Entity e, const GridPos& p, bool updateSpatialPosition) {
    auto it = cells.find(p);
    if (it == cells.end())
        LOGF("Tried to add entity: '" << theEntityManager.entityName(e) << " at invalid pos: " << p);

    it->second.entities.push_back(e);
    entityToGridPos[e].push_back(p);

    if (updateSpatialPosition) {
        TRANSFORM(e)->position = gridPosToPosition(p);
    }
}

void HexSpatialGrid::removeEntityFrom(Entity e, const GridPos& p) {
    auto it = cells.find(p);
    if (it == cells.end())
        LOGF("Tried to remove entity: '" << theEntityManager.entityName(e) << " at invalid pos: " << p);
    it->second.entities.remove(e);
    entityToGridPos[e].remove(p);
}

std::list<Entity>& HexSpatialGrid::getEntitiesAt(const GridPos& p) {
    auto it = cells.find(p);
    if (it == cells.end())
        LOGF("Tried to get entities at invalid pos: '" << p.q << "," << p.r << "'");

    std::list<Entity>& result = it->second.entities;
#if SAC_DEBUG
    for (Entity e: result) {
        auto a = entityToGridPos.find(e);
        LOGF_IF(a == entityToGridPos.end(), "Entity '" << e << "' not in map");
        LOGF_IF(std::find(a->second.begin(), a->second.end(), p) == a->second.end(), "Entity/Gridpos inconsistent '" << e << "' / " << p);
    }
#endif
    return result;
}

void HexSpatialGrid::autoAssignEntitiesToCell(const std::vector<Entity>& entities) {
    const Polygon hexagon = Polygon::create(Shape::Hexagon);

    for (auto e: entities) {
        // Clear current position(s)
        auto currentPos = entityToGridPos.find(e);
        if (currentPos != entityToGridPos.end()) {
            auto positions = currentPos->second;
            for (auto p: positions) {
                removeEntityFrom(e, p);
            }
        }

        auto* trans = TRANSFORM(e);
        auto* gr = theGridSystem.Get(e, false);

        for (auto& a: cells) {
            const GridPos& p = a.first;
            const glm::vec2 center = gridPosToPosition(p);

            // If the center of the cell is in the entity -> entity is inside
            bool isInside =
                IntersectionUtil::pointRectangle(center,
                    trans->position, trans->size, trans->rotation);

            // If entity covers more than 2 vertices -> inside
            if (!isInside) {
                int count = 0;
                for (int i=1; i<=6 && count < 3; i++) {
                    if (IntersectionUtil::pointRectangle(center + hexagon.vertices[i] * size, trans->position, trans->size, trans->rotation)) {
                        count++;
                    }
                }
                isInside = (count >= 3);
            }

            if (isInside) {
                this->addEntityAt(e, p);
                // snap position
                if (gr && !gr->canBeOnMultipleCells) {
                    trans->position = center;
                    break;
                }
            }
        }
    }
}

SpatialGrid::Iterate::Result HexSpatialGrid::iterate(GridPos pos, SpatialGrid::Iterate::Options ) const {
    Iterate::Result result;
    result.newLine = false;

    if (!isPosValid(pos)) {
        // return top-left corner
        result.valid = true;
        int min = INT_MAX;
        for (const auto& p: cells) {
            if (p.first.q < min) {
                min = p.first.q;
                result.pos = p.first;
            }
        }
        return result;
    } else {
        GridPos ret = pos;
        ret.q += 1;
        // if we reach the end of the line
        if (!isPosValid(ret)) {
            ret = pos;
            ret.q = ret.q - w + 1;
            // try bottom-left
            ret.r -= 1;
            result.newLine = true;
            if (!isPosValid(ret)) {
                // try bottom right
                ret.q += 1;

                if (!isPosValid(ret)) {
                    result.valid = false;
                    return result;
                }
            }
        }
        result.pos = ret;
        result.valid = true;
    }
    return result;
}

std::map<int, std::vector<GridPos> > HexSpatialGrid::movementRange(const GridPos& p, int movement) const {

    auto it = cells.find(p);

    LOGF_IF(it == cells.end(), "Tried to find movement range at invalid position: '" << p.q << "," << p.r <<"'");

    std::map<GridPos, int> visited;
    std::list<GridPos> lastlyAdded;

    visited.insert(std::make_pair(p, 0));
    lastlyAdded.push_back(p);

    while (!lastlyAdded.empty()) {
        // browse last added positions
        auto gp = lastlyAdded.front();
        lastlyAdded.pop_front();

        auto v = visited.find(gp);

        const int movePointLeft = movement - (*v).second;

        if (movePointLeft > 0) {
            std::vector<GridPos> neighbors = getNeighbors(gp);

            for (const auto& npos: neighbors) {
                // do not add neighboor if it's blocking
                if (isPathBlockedAt(npos)) {
                    continue;
                }
                const int travelCost = (*v).second + gridPosMoveCost(v->second, npos);
                const int leftAfterMove = movement - travelCost;

                if (leftAfterMove >= 0) {
                    // do not add neighboor if already added with lower cost
                    const auto ex = visited.find(npos);
                    if (ex != visited.end() && (*ex).second <= travelCost) {
                        continue;
                    }

                    visited[npos] = travelCost;
                    if (leftAfterMove > 0)
                        lastlyAdded.push_back(npos);
                }
            }
        }
    }

    std::map<int, std::vector<GridPos> > range;
    for (const auto& v: visited) {
        range[v.second].push_back(v.first);
    }

    return std::move(range);
}

std::vector<GridPos> HexSpatialGrid::viewRange(const GridPos& position, int size) const {
    std::vector<GridPos> range, visibleButBlocking;

    std::vector<GridPos> borderLine = ringFinder(position, size, true);

    // We draw line between position and all border point to make "ray casting"
    for (auto point: borderLine) {
        std::vector<GridPos> ray = lineDrawer(position, point);
        for (auto r: ray) {
            // if the point is out of grid we pass this point. We can probably break the loop here
            if (!isPosValid(r))
                continue;
            bool isVisited = false;
            //Check if the point is already in the output vector
            for (auto v: range){
                if (r == v){
                    isVisited = true;
                    break;
                }
            }
            // If not, we check if it's block
            if (!isVisited) {
                if (isVisibilityBlockedAt(r)) {
                    if (isPathBlockedAt(r)) {
                        visibleButBlocking.push_back(r);
                    }
                    break;
                } else {
                    range.push_back(r);
                }
            }
        }
    }
    std::copy(visibleButBlocking.begin(), visibleButBlocking.end(), std::back_inserter(range));
    return std::move(range);
}

std::vector<GridPos> HexSpatialGrid::lineDrawer(const GridPos& from, const GridPos& to, bool positiveEps) const {
    std::vector<GridPos> line;


    float E = (float)1e-6 * (positiveEps ? 1.f : -1.f);

    float fromQ = from.q + E;
    float fromR = from.r - 2 * E;

    float dx = fromQ - to.q;
    //since x(q)+y+z(r)=0, y = - (x + z) = - (q + r)
    float dy = - (fromQ + fromR) - ( - (to.q + to.r));
    float dz = fromR - to.r;

    float N = glm::max(glm::max(glm::abs(dx-dy), glm::abs(dy-dz)), glm::abs(dz-dx));

    GridPos prev(99,99);
    for (int i = 0; i <= N; ++i) {
        const float iOverN = i / N;

        const float x = fromQ - iOverN * dx; // <=> fromX + iOverN * (endX - fromX)
        const float y = - (fromQ + fromR) - iOverN * dy; // <=> fromX + iOverN * (endX - fromX)
        const float z = fromR - iOverN * dz; // <=> fromX + iOverN * (endX - fromX)

        GridPos p = cubeCoordinateRounding(x, y, z);
        if (p != prev) {
            line.push_back(p);
            prev = p;
        }
    }
    return std::move(line);
}

int HexSpatialGrid::canDrawLine(const GridPos& p1, const GridPos& p2) const {
    auto line = lineDrawer(p1, p2);

    for (auto& gp: line) {
        if (!(gp == p2 || gp == p1) && isPathBlockedAt(gp)) {
            return -1;
        }
    }
    return line.size();
}

std::vector<GridPos> HexSpatialGrid::ringFinder(const GridPos& pos, int range, bool enableInvalidPos = false) const {
    std::vector<GridPos> ring;

    auto it = cells.find(pos);
    LOGF_IF(it == cells.end(), "Tried to find movement range at invalid position: '" << pos.q << "," << pos.r <<"'");

    GridPos p(pos.q-range, pos.r+range);

    for(int i=0; i<6; ++i) {
        for (int j=0; j<range; ++j) {
            if (enableInvalidPos || isPosValid(p))
                ring.push_back(p);
            else
                LOGI("Invalid position : '"<< p.q << "," << p.r << "'");
            p = getNeighbors(p, true)[i];
        }
    }
    return std::move(ring);
}

std::vector<GridPos> HexSpatialGrid::findPath(const GridPos& from, const GridPos& to, bool ignoreBlockedEndPath) const {
    struct Node {
        int rank, cost;
        GridPos pos;
        std::list<Node>::iterator parent;

        Node(GridPos _pos, int _rank = 0, int _cost = 0)
            : rank(_rank), cost(_cost), pos(_pos) { }
    };

    std::list<Node> open, closed;
    std::list<Node>::iterator success = open.end();

    // Initialize with 'from' position
    Node root(from, 0, 0);
    root.parent = closed.end();
    open.push_back(root);
    do {
        // Pick best open node
        int min = 10000;
        std::list<Node>::iterator best = open.end();
        for (auto it = open.begin(); it!=open.end(); ++it) {
            if ((*it).rank < min) {
                min = (*it).rank;
                best = it;
            }
        }
        LOGF_IF(best == open.end(), "No open node found");

        // Is it the goal ?
        if ((*best).pos == to) {
            success = best;
            break;
        } else {
            Node current = *best;

            LOGV(2, "Current: " << current.pos << ", " << current.rank << ", " << current.cost);
            LOGV(2, "Open: " << open.size() << ", closed: " << closed.size());
            // Remove from open
            open.erase(best);
            // Add to closed
            auto pIt = closed.insert(closed.begin(), current);

            // Browse neighbors
            std::vector<GridPos> neighbors = getNeighbors(current.pos);
            for (GridPos& n: neighbors) {
                if (isPathBlockedAt(n) && !(ignoreBlockedEndPath && n == to))
                    continue;
                int cost = current.cost + gridPosMoveCost(current.pos, n);

                // Is n already in closed ?
                auto jt = std::find_if(closed.begin(), closed.end(), [n] (const Node& node) -> bool {
                    return node.pos == n;
                });
                if (jt != closed.end()) {
                    LOGT("Compare cost, and keep node if it can be improved");
                    continue;
                }

                // Is n already in open ?
                jt = std::find_if(open.begin(), open.end(), [n] (const Node& node) -> bool {
                    return node.pos == n;
                });
                // If node is in open && new cost >= existing cost -> skip node
                if ((jt != open.end()) && (cost >= (*jt).cost)) {
                    continue;
                }
                // Else insert/replace node
                if (jt == open.end()) {
                    jt = open.insert(open.begin(), Node(0, 0));
                }
                (*jt).pos = n;
                (*jt).cost = cost;
                (*jt).rank = cost + computeGridDistance(n, to);
                (*jt).parent = pIt;
            }
        }
    } while (!open.empty());

    LOGE_IF(success == open.end(), "Couldn't find a path: " << from << " -> " << to);
    if (success == open.end())
        return std::vector<GridPos>();

    LOGV(1, "Found path: " << from << " -> " << to << ':');
    std::vector<GridPos> result;
    do {
        result.push_back((*success).pos);
        success = (*success).parent;
    } while ((*success).parent != closed.end());
    std::reverse(result.begin(), result.end());

#if SAC_ENABLE_LOG
    for (const auto& gp: result) {
            LOGV(1, "   - " << gp);
    }
#endif
    return result;
}

GridPos HexSpatialGrid::cubeCoordinateRounding(float x, float y, float z)  const {
    float rx = glm::round(x);
    float ry = glm::round(y);
    float rz = glm::round(z);
    float x_err = glm::abs(rx - x);
    float y_err = glm::abs(ry - y);
    float z_err = glm::abs(rz - z);

    if (x_err > y_err && x_err > z_err) {
            rx = -ry - rz;
    } else if (y_err > z_err) {
            ry = -rx - rz;
    } else {
            rz = -rx - ry;
    }

    return GridPos(rx, rz);
}

GridPos HexSpatialGrid::positionSizeToGridPos(const glm::vec2& pos/*, float size*/) const {
        // the center of the grid is at 0,0
        float q = (1.0f/3 * glm::sqrt(3.0f) * pos.x - 1.0f/3 * pos.y) / size;
        float r = 2.0f/3 * pos.y / size;

        return cubeCoordinateRounding(q, 0 - (q + r), r);
}

AABB HexSpatialGrid::boundingBox(bool inner) const {
    AABB boundingBox;

    // float hexaHeight = hexagonWidth / (glm::sqrt(3.0f) * 0.5f);
    // size = hexaHeight * 0.5f;

    boundingBox.bottom  = - 2.6 * h / 2.f;
    boundingBox.top     = - boundingBox.bottom;
    boundingBox.left    = - 2.6 * w / 2.f;
    boundingBox.right   = - boundingBox.left;
    return boundingBox;
}

#endif
