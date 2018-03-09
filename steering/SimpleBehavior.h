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

#pragma once

#include "base/Interval.h"

namespace Steering {
    struct SeekParams {
        SeekParams() : weight(0) {}
        Entity* entities;
        float* weight;
        int count;
    };

    struct FleeParams {
        float radius;
        Entity target;
    };

    struct AvoidParams {
        Entity* entities;
        int count;
    };

    struct SeparationParams {
        Entity* entities;
        int count;
        float radius;
    };

    struct AlignmentParams {
        Entity* entities;
        int count;
        float radius;
    };

    struct CohesionParams {
        Entity* entities;
        int count;
        float radius;
    };

    struct GroupParams {
        Entity* entities;
        int count;
        float neighborRadius;
    };

    struct ArriveParams {
        Entity target;
        float breakingDistance;
    };

    struct WanderParams {
        WanderParams() : change(0) {}
        float distance, radius, jitter;
        glm::vec2 target;
        Interval<float> pauseDuration;
        float change;
    };
}
