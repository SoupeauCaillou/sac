#pragma once

#include "System.h"
#include <vector>

struct DebuggingComponent {
};

#define theDebuggingSystem DebuggingSystem::GetInstance()
#define DEBUGGING(e) theDebuggingSystem.Get(e)

UPDATABLE_SYSTEM(Debugging)

    public:
        void toggle();
        void drawVector(glm::vec2 position, glm::vec2 size);
    private:
        bool enable;
        std::map<std::string, Entity> debugEntities;
        std::vector<Entity> renderStatsEntities;

        Entity fps, entityCount, systems;
        Entity fpsLabel, entityCountLabel;

        std::list<Entity> vectorList;
};
