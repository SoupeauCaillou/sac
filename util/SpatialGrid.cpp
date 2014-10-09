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

#if !DISABLE_GRID_SYSTEM

#include "SpatialGrid.h"

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

static uint64_t Hash(const GridPos& p) {
    uint64_t h = (uint64_t)p.q;
    h <<= 32;
    return h | (uint32_t)p.r;
}

bool GridPos::operator<(const GridPos& p) const {
    return Hash(p) < Hash(*this);
}
bool GridPos::operator==(const GridPos& p) const {
    return Hash(p) == Hash(*this);
}
bool GridPos::operator!=(const GridPos& p) const {
    return Hash(p) != Hash(*this);
}

std::ostream& operator<<(std::ostream& str, const GridPos& gp) {
    str << '{' << gp.q <<',' << gp.r << '}';
    return str;
}


/*
static GridPos DeHash(uint64_t p) {
    GridPos g;
    g.q = (int32_t)(p >> 32 & 0xffffffff);
    g.r = (int32_t)(p & 0xffffffff);
    return g;
}
*/

GridPos::GridPos(int32_t pQ, int32_t pR) : q(pQ), r(pR) {

}

#endif
