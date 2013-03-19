#pragma once

#include "System.h"

struct DebuggingComponent {
};

#define theDebuggingSystem DebuggingSystem::GetInstance()
#define DEBUGGING(e) theDebuggingSystem.Get(e)

UPDATABLE_SYSTEM(Debugging)

    public:
        bool addDebugEntity(std::string debugName, Color lineColor,
                            float maxY, float minY, bool newGraph,
                            std::string parentGraphName);
        bool addDebugEntity(std::string debugName, Entity debugEntity);
        void deleteDebugEntity(std::string debugName);
        void addValue(std::string debugEntity, std::pair<float, float> value);
        void clearDebuggingEntity(std::string debugEntity);

    private:
        std::map<std::string, Entity> debugEntities;

        std::vector<Entity> captionGraph;

        Entity activeCamera;
};
