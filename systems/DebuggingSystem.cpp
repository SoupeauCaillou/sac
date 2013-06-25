#include "DebuggingSystem.h"
#include "CameraSystem.h"
#include "TransformationSystem.h"
#include "AnchorSystem.h"
#include "RenderingSystem.h"
#include "GraphSystem.h"
#include "TextSystem.h"
#include <iomanip>

#include <base/EntityManager.h>

#if SAC_DEBUG
static const char* FpsTextureName = "__debug_fps_texture";
static const char* EntitiesTextureName = "__debug_entities_texture";
static const char* SystemsTextureName = "__debug_systems_texture";

static float timeUntilGraphUpdate = 0;

static Entity createSystemGraphEntity(const std::string& name, Entity parent, int index, const std::string& s, float maxY = 0);
#endif

static unsigned long frameCount = 0;


INSTANCE_IMPL(DebuggingSystem);

DebuggingSystem::DebuggingSystem() : ComponentSystemImpl<DebuggingComponent>("Debugging") {
    fps = entityCount = systems = fpsLabel = entityCountLabel = 0;
    frameCount = 0;
    enable = false;
}

#if ! SAC_DEBUG
void DebuggingSystem::DoUpdate(float dt) {}
void DebuggingSystem::toggle() {}

#else
static void init(Entity camera, Entity& fps, Entity& fpsLabel, Entity& entityCount, Entity& entityCountLabel, Entity& systems) {
    const glm::vec2& cameraSize = TRANSFORM(camera)->size;

    fps = theEntityManager.CreateEntity("__debug_fps");
    ADD_COMPONENT(fps, Transformation);
    TRANSFORM(fps)->size = cameraSize * glm::vec2(0.3f, 0.2f);
    ADD_COMPONENT(fps, Anchor);
    ANCHOR(fps)->parent = camera;
    ANCHOR(fps)->position = cameraSize * glm::vec2(-0.5f, 0.5f) + TRANSFORM(fps)->size * glm::vec2(0.5f, -0.5f);
    ANCHOR(fps)->z = 1 - TRANSFORM(camera)->z;
    ADD_COMPONENT(fps, Rendering);
    RENDERING(fps)->texture = theRenderingSystem.loadTextureFile(FpsTextureName);
    RENDERING(fps)->show = true;
    ADD_COMPONENT(fps, Graph);
    GRAPH(fps)->textureName = FpsTextureName;
    GRAPH(fps)->minY = 0;
    GRAPH(fps)->maxY = 65;

    fpsLabel = theEntityManager.CreateEntity("__debug_label_fps");
    ADD_COMPONENT(fpsLabel, Transformation);
    TRANSFORM(fpsLabel)->size = TRANSFORM(fps)->size;
    ADD_COMPONENT(fpsLabel, Anchor);
    ANCHOR(fpsLabel)->parent = fps;
    ANCHOR(fpsLabel)->z = -0.002;

    ADD_COMPONENT(fpsLabel, Text);
    TEXT(fpsLabel)->positioning = TextComponent::LEFT;
    // TEXT(fpsLabel)->flags = TextComponent::AdjustHeightToFillWidthBit;
    TEXT(fpsLabel)->maxCharHeight = 0.4;
    TEXT(fpsLabel)->show = true;

    entityCount = theEntityManager.CreateEntity("__debug_entityCount");
    ADD_COMPONENT(entityCount, Transformation);
    TRANSFORM(entityCount)->size = cameraSize * glm::vec2(0.3f, 0.2f);
    ADD_COMPONENT(entityCount, Anchor);
    ANCHOR(entityCount)->parent = camera;
    ANCHOR(entityCount)->position = cameraSize * glm::vec2(0.0f, 0.5f) + TRANSFORM(fps)->size * glm::vec2(0.0f, -0.5f);
    ANCHOR(entityCount)->z = 1 - TRANSFORM(camera)->z;
    ADD_COMPONENT(entityCount, Rendering);
    RENDERING(entityCount)->texture = theRenderingSystem.loadTextureFile(EntitiesTextureName);
    RENDERING(entityCount)->show = true;
    ADD_COMPONENT(entityCount, Graph);
    GRAPH(entityCount)->textureName = EntitiesTextureName;

    entityCountLabel = theEntityManager.CreateEntity("__debug_label_entityCount");
    ADD_COMPONENT(entityCountLabel, Transformation);
    TRANSFORM(entityCountLabel)->size = TRANSFORM(entityCount)->size;
    ADD_COMPONENT(entityCountLabel, Anchor);
    ANCHOR(entityCountLabel)->parent = entityCount;
    ANCHOR(entityCountLabel)->z = -0.002;

    ADD_COMPONENT(entityCountLabel, Text);
    TEXT(entityCountLabel)->positioning = TextComponent::LEFT;
    // TEXT(entityCountLabel)->flags = TextComponent::AdjustHeightToFillWidthBit;
    TEXT(entityCountLabel)->maxCharHeight = 0.4;
    TEXT(entityCountLabel)->show = true;

    systems = theEntityManager.CreateEntity("__debug_systems");
    ADD_COMPONENT(systems, Transformation);
    TRANSFORM(systems)->size = cameraSize * glm::vec2(0.3f, 0.2f);
    ADD_COMPONENT(systems, Anchor);
    ANCHOR(systems)->parent = camera;
    ANCHOR(systems)->position = cameraSize * glm::vec2(0.5f, 0.5f) + TRANSFORM(fps)->size * glm::vec2(-0.5f, -0.5f);
    ANCHOR(systems)->z = 1 - TRANSFORM(camera)->z;
    ADD_COMPONENT(systems, Rendering);
    RENDERING(systems)->texture = theRenderingSystem.loadTextureFile(SystemsTextureName);
    RENDERING(systems)->show = true;
}

static Entity createSystemGraphEntity(const std::string& name, Entity parent, int index, const std::string& textureName, float maxY) {
    Color color = Color::random();
    color.a = 1;
    Entity e = theEntityManager.CreateEntity(std::string("__debug_") + name);
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->size = TRANSFORM(parent)->size;
    ADD_COMPONENT(e, Anchor);
    ANCHOR(e)->parent = parent;
    ANCHOR(e)->z = -0.002;
    ANCHOR(e)->position = TRANSFORM(parent)->size * (-0.5f) - glm::vec2(0.0f, (index + 1) * 0.6f);

    ADD_COMPONENT(e, Text);
    TEXT(e)->color = color;
    TEXT(e)->positioning = TextComponent::LEFT;
    // TEXT(e)->flags = TextComponent::AdjustHeightToFillWidthBit;
    TEXT(e)->maxCharHeight = 0.4;
    TEXT(e)->text = name;
    TEXT(e)->show = true;

    ADD_COMPONENT(e, Graph);
    GRAPH(e)->textureName = textureName;
    GRAPH(e)->minY = 0;
    GRAPH(e)->maxY = maxY;
    GRAPH(e)->lineColor = color;

    return e;
}

template <class T, class U>
std::string createLabel(const std::string& title, const std::list<std::pair<T, U> > pointsList, float scale, const std::string& unit, int optionalCount = -1) {
    U minDt, maxDt, avg = 0;
    minDt = maxDt = pointsList.front().second;
    std::for_each(pointsList.begin(), pointsList.end(), [&minDt, &maxDt, &avg] (std::pair<T, U> pt) -> void {
        const U t = pt.second;
        if (t < minDt) minDt = t;
        if (t > maxDt) maxDt = t;
        avg += t;
    });
    avg /= pointsList.size();
    std::stringstream ss;
    ss << title <<": " << std::fixed << std::setprecision(1) << scale * avg << ' ' << scale * minDt << ' ' << scale * maxDt << ' ' << unit;

    if (optionalCount >= 0) {
        ss << " (" << optionalCount << ')';
    }
    return ss.str();
}

void DebuggingSystem::toggle() {
    enable = !enable;
    LOGI("Debugging " << (enable ? "enabled" : "disabled"));
    for (auto it: debugEntities)
        TEXT(it.second)->show = enable;
    for (auto it: renderStatsEntities)
        TEXT(it)->show = enable;

    if (fps) RENDERING(fps)->show = enable;
    if (entityCount) RENDERING(entityCount)->show = enable;
    if (systems) RENDERING(systems)->show = enable;
    if (fpsLabel) TEXT(fpsLabel)->show = enable;
    if (entityCountLabel) TEXT(entityCountLabel)->show = enable;
}
void DebuggingSystem::DoUpdate(float dt) {
    if (!enable) {
        return;
    }

    frameCount++;
    timeUntilGraphUpdate -= dt;

    Entity activeCamera = 0;
    std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    if (cameras.empty()) {
        return;
    }

    for (std::vector<Entity>::iterator it = cameras.begin(); it != cameras.end(); ++it) {
        if (CAMERA(*it)->fb == DefaultFrameBufferRef) {
            activeCamera = *it;
            break;
        }
    }

    if (!fps) {
        init(activeCamera, fps, fpsLabel, entityCount, entityCountLabel, systems);
        renderStatsEntities.push_back(createSystemGraphEntity("opaque_object", fps, 1, FpsTextureName));
        renderStatsEntities.push_back(createSystemGraphEntity("nonopaque_object", fps, 2, FpsTextureName));
        renderStatsEntities.push_back(createSystemGraphEntity("zprepass_object", fps, 3, FpsTextureName));
        renderStatsEntities.push_back(createSystemGraphEntity("opaque_area", fps, 4, FpsTextureName));
        renderStatsEntities.push_back(createSystemGraphEntity("nonopaque_area", fps, 5, FpsTextureName));
        renderStatsEntities.push_back(createSystemGraphEntity("zprepass_area", fps, 6, FpsTextureName));
        LOGI("Initialize DebugSystem: " << fps << ", " << entityCount << ", " << systems);
    }
    const glm::vec2 firstLabelOffset(TRANSFORM(activeCamera)->size * glm::vec2(0.3, 0.2) * (-0.5f) - glm::vec2(0, (0.6) * 0.6f));
    const glm::vec2 labelsSpacing(0, -0.6);

    bool reloadTextures = (timeUntilGraphUpdate < 0);

    // Fill FPS graph
    GRAPH(fps)->pointsList.push_back(std::make_pair(frameCount, 1.0/dt));
    if (frameCount > 120) GRAPH(fps)->pointsList.pop_front();
    GRAPH(fps)->reloadTexture = reloadTextures;

    // Fill Entity count graph
    GRAPH(entityCount)->pointsList.push_back(std::make_pair(frameCount, theEntityManager.getNumberofEntity()));
    if (frameCount > 120) GRAPH(entityCount)->pointsList.pop_front();
    GRAPH(entityCount)->reloadTexture = reloadTextures;

    for (int i=0; i<3; i++) {
        GRAPH(renderStatsEntities[i])->pointsList.push_back(std::make_pair(frameCount, theRenderingSystem.renderingStats[i].count));
        if (frameCount > 120) GRAPH(renderStatsEntities[i])->pointsList.pop_front();
        GRAPH(renderStatsEntities[i])->reloadTexture = reloadTextures;

        GRAPH(renderStatsEntities[3 + i])->pointsList.push_back(std::make_pair(frameCount, theRenderingSystem.renderingStats[i].area));
        if (frameCount > 120) GRAPH(renderStatsEntities[3 + i])->pointsList.pop_front();
        GRAPH(renderStatsEntities[3 + i])->reloadTexture = reloadTextures;
    }

    if (reloadTextures) {
        TEXT(fpsLabel)->text = createLabel("FPS", GRAPH(fps)->pointsList, 1, "fps");
        TEXT(renderStatsEntities[0])->text = createLabel("Opaque", GRAPH(renderStatsEntities[0])->pointsList, 1, " drawn");
        TEXT(renderStatsEntities[1])->text = createLabel("NonOpaque", GRAPH(renderStatsEntities[1])->pointsList, 1, " drawn");
        TEXT(renderStatsEntities[2])->text = createLabel("Zprepass", GRAPH(renderStatsEntities[2])->pointsList, 1, " drawn");
        TEXT(renderStatsEntities[3])->text = createLabel("OpSurf", GRAPH(renderStatsEntities[3])->pointsList, 1, " pct");
        TEXT(renderStatsEntities[4])->text = createLabel("NonOpSurf", GRAPH(renderStatsEntities[4])->pointsList, 1, " pct");
        TEXT(renderStatsEntities[5])->text = createLabel("ZppSurf", GRAPH(renderStatsEntities[5])->pointsList, 1, " pct");
        TRANSFORM(fpsLabel)->position = firstLabelOffset;
        TEXT(entityCountLabel)->text = createLabel("Total", GRAPH(entityCount)->pointsList, 1, "entities");
        TRANSFORM(entityCountLabel)->position = firstLabelOffset;
    }


    std::vector<std::string> systemNames = ComponentSystem::registeredSystemNames();
    // sort from highest time consumer to lowest
    std::sort(systemNames.begin(), systemNames.end(),
        [this](const std::string& s1, const std::string& s2) -> bool {
            const ComponentSystem* system1 = ComponentSystem::Named(s1);
            const ComponentSystem* system2 = ComponentSystem::Named(s2);
            return system1->updateDuration > system2->updateDuration;
        }
    );
    for (unsigned i=0; i<systemNames.size(); i++) {
        const ComponentSystem* system = ComponentSystem::Named(systemNames[i]);

        auto it = debugEntities.find(systemNames[i]);
        if (it == debugEntities.end()) {
            it = debugEntities.insert(std::make_pair(systemNames[i], createSystemGraphEntity(systemNames[i], systems, debugEntities.size(), SystemsTextureName, 0.020))).first;
        }
        Entity e = it->second;
        GraphComponent* graphC = GRAPH(e);
        graphC->pointsList.push_back(std::make_pair(frameCount, system->updateDuration));
        if (frameCount > 120) graphC->pointsList.pop_front();
        graphC->reloadTexture = reloadTextures;

        // only display system which takes >= .1 ms to update
        if (reloadTextures) {
            if (system->updateDuration < 0.0001) {
                TEXT(e)->show = false;
            } else {
                TEXT(e)->text = createLabel(systemNames[i], graphC->pointsList, 1000, "ms", system->entityCount());
                TEXT(e)->show = true;
                TRANSFORM(e)->position = firstLabelOffset + labelsSpacing * (float)i;
            }
        }
    }


    timeUntilGraphUpdate += reloadTextures * .5;

#if 0


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
        TRANSFORM(c)->size = glm::vec2(10, 1);
        TRANSFORM(c)->position = glm::vec2(0, -1);

        ADD_COMPONENT(c, Text);
        TEXT(c)->fontName = "typo";
        TEXT(c)->show = false;
        TEXT(c)->charHeight = 1;
        captionGraph.push_back(c);
    }

    if (activeCamera && captionGraph.size()) {
        TRANSFORM(captionGraph.front())->parent = activeCamera;
        TRANSFORM(captionGraph.front())->position.X = (-1 * TRANSFORM(activeCamera)->size.X/2) + TRANSFORM(captionGraph.front())-> size.X / 2;
        TRANSFORM(captionGraph.front())->position.Y = (TRANSFORM(activeCamera)->size.Y/2) - TRANSFORM(captionGraph.front())-> size.Y / 2;
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

                TRANSFORM(*it)->size.X = a.str().size()*0.5;
                TRANSFORM(*it)->z = 0;
                if (it == captionGraph.begin() ){
                    TRANSFORM(*it)->z = -0.1;
                    TRANSFORM(*it)->parent = activeCamera;
                }
                TEXT(*it)->show = true;
                TEXT(*it)->text = a.str();
                TEXT(*it)->color = GRAPH(debugEntity.second)->lineColor;

                pit = it;
                ++it;
            }
        }
    }

    if (reloadFrequency > 0.5f) {
        reloadFrequency = 0;
        while (it != captionGraph.end()) {
            TEXT(*it)->show = false;
            ++it;
        }
    }
#endif
}
#endif
