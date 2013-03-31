#pragma once

#include "System.h"
#include <vector>

struct DebuggingComponent {
};

#define theDebuggingSystem DebuggingSystem::GetInstance()
#define SAC_DEBUGGING(e) theDebuggingSystem.Get(e)

UPDATABLE_SYSTEM(Debugging)

    private:
        std::map<std::string, Entity> debugEntities;
        std::vector<Entity> renderStatsEntities;

        Entity fps, entityCount, systems;
        Entity fpsLabel, entityCountLabel;
};
