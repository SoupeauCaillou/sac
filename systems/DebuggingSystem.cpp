#include "DebuggingSystem.h"
#include "CameraSystem.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"
#include "GraphSystem.h"
#include "TextRenderingSystem.h"

INSTANCE_IMPL(DebuggingSystem);

DebuggingSystem::DebuggingSystem() : ComponentSystemImpl<DebuggingComponent>("Debugging") {
    activeCamera = 0;
}

bool DebuggingSystem::addDebugEntity(std::string debugName, Color lineColor=Color(1,1,1),
                                    float maxY=0, float minY=0, bool newGraph=true,
                                    std::string parentGraphName="") {

    if (newGraph) {
        Entity de = theEntityManager.CreateEntity(debugName);
        ADD_COMPONENT(de, Transformation);
        TRANSFORM(de)->size = glm::vec2(10.0f);
        TRANSFORM(de)->position = glm::vec2(0.0f);
        TRANSFORM(de)->parent = activeCamera;
        ADD_COMPONENT(de, Rendering);
        RENDERING(de)->hide = false;
        RENDERING(de)->texture = theRenderingSystem.loadTextureFile(debugName);
        ADD_COMPONENT(de, Graph);
        GRAPH(de)->textureName = debugName;
        GRAPH(de)->lineColor = lineColor;
        GRAPH(de)->maxY = maxY;
        GRAPH(de)->minY = minY;
        debugEntities[debugName] = de;

    } else {
        Entity parent = debugEntities[parentGraphName];
        if (parent) {
            Entity de = theEntityManager.CreateEntity(debugName);
            ADD_COMPONENT(de, Graph);
            GRAPH(de)->textureName = GRAPH(parent)->textureName;
            GRAPH(de)->lineColor = lineColor;
            debugEntities[debugName] = de;
            GRAPH(de)->maxY = maxY;
            GRAPH(de)->minY = minY;
        }
        else
            return false;
    }
    return true;
}
bool DebuggingSystem::addDebugEntity(std::string debugName, Entity debugEntity) {
    if (theGraphSystem.Get(debugEntity, false) &&
        theTransformationSystem.Get(debugEntity, false)) {
        if (debugEntities.count(debugName) == 0) {
            debugEntities[debugName] = debugEntity;
            return true;
        }
    }
    return false;
}

void DebuggingSystem::deleteDebugEntity(std::string debugName) {
    if (debugEntities.count(debugName)) {
        theEntityManager.DeleteEntity(debugEntities[debugName]);
        debugEntities.erase(debugName);
    }
}

void DebuggingSystem::DoUpdate(float dt) {

    if (!activeCamera) {
        std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
        if (!cameras.empty()) {
            for (std::vector<Entity>::iterator it = cameras.begin(); it != cameras.end(); ++it) {
                if (CAMERA(*it)->fb == DefaultFrameBufferRef) {
                    activeCamera = *it;
                    break;
                }
            }
        }
    }

    while (debugEntities.size() * 2 < captionGraph.size()) {
        theEntityManager.DeleteEntity(captionGraph.back());
        captionGraph.pop_back();
    }

    for (uint i = captionGraph.size(); i < debugEntities.size(); ++i) {
        Entity c = theEntityManager.CreateEntity("caption");
        ADD_COMPONENT(c, Transformation);
        if (i==0) {
            TRANSFORM(c)->parent = activeCamera;
        } else {
            TRANSFORM(c)->parent = captionGraph[i-1];
        }
        TRANSFORM(c)->size = glm::vec2(10.0f, 1.0f);
        TRANSFORM(c)->position = glm::vec2(0.0f, -1.0f);

        ADD_COMPONENT(c, TextRendering);
        TEXT_RENDERING(c)->fontName = "typo";
        TEXT_RENDERING(c)->hide = true;
        TEXT_RENDERING(c)->charHeight = 1;
        captionGraph.push_back(c);
    }

    if (activeCamera && captionGraph.size()) {
        TRANSFORM(captionGraph.front())->parent = activeCamera;
        TRANSFORM(captionGraph.front())->position.x = (-1 * TRANSFORM(activeCamera)->size.x/2) + TRANSFORM(captionGraph.front())-> size.x / 2;
        TRANSFORM(captionGraph.front())->position.y = (TRANSFORM(activeCamera)->size.y/2) - TRANSFORM(captionGraph.front())-> size.y / 2;
    }

    static float reloadFrequency = 0.6;
    reloadFrequency += dt;

    std::vector<Entity>::iterator it = captionGraph.begin(), pit = it;
    for (auto debugEntity : debugEntities) {
        if (activeCamera) {
            if (TransformationComponent* tc = theTransformationSystem.Get(debugEntity.second, false))
                tc->parent = activeCamera;
        }

        while (GRAPH(debugEntity.second)->pointsList.size() > 60) GRAPH(debugEntity.second)->pointsList.pop_front();

        if (reloadFrequency > 0.5f) {
            GRAPH(debugEntity.second)->reloadTexture= true;

            if (captionGraph.size() > 0) {
                std::stringstream a;
                std::pair<float, float> p = GRAPH(debugEntity.second)->pointsList.back();
                a << debugEntity.first << " X=" << p.first << " Y=" << p.second;

                TRANSFORM(*it)->size.x = a.str().size()*0.5;
                TRANSFORM(*it)->z = 0;
                if (it == captionGraph.begin() ){
                    TRANSFORM(*it)->z = -0.1;
                    TRANSFORM(*it)->parent = activeCamera;
                }
                TEXT_RENDERING(*it)->hide = false;
                TEXT_RENDERING(*it)->text = a.str();
                TEXT_RENDERING(*it)->color = GRAPH(debugEntity.second)->lineColor;

                pit = it;
                ++it;
            }
        }
    }

    if (reloadFrequency > 0.5f) {
        reloadFrequency = 0;
        while (it != captionGraph.end()) {
            TEXT_RENDERING(*it)->hide = true;
            ++it;
        }
    }
}

void DebuggingSystem::addValue(std::string debugEntity, std::pair<float, float> value) {
    if (debugEntities.count(debugEntity)) {
        GRAPH(debugEntities[debugEntity])->pointsList.push_back(value);
    }
}

void DebuggingSystem::clearDebuggingEntity(std::string debugEntity) {
    if (debugEntities.count(debugEntity)) {
        GRAPH(debugEntities[debugEntity])->pointsList.clear();
    }
}

#ifdef INGAME_EDITORS
void DebuggingSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    DebuggingComponent* dc = Get(entity, false);
    if (!dc) return;
}
#endif
