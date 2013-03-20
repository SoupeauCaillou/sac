#include "DebuggingSystem.h"
#include "CameraSystem.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"
#include "GraphSystem.h"
#include "TextRenderingSystem.h"

static const char* FpsTextureName = "__debug_fps_texture";
static const char* EntitiesTextureName = "__debug_entities_texture";
static const char* SystemsTextureName = "__debug_systems_texture";

static unsigned long frameCount = 0;
static float timeUntilGraphUpdate = 0;
INSTANCE_IMPL(DebuggingSystem);

DebuggingSystem::DebuggingSystem() : ComponentSystemImpl<DebuggingComponent>("Debugging") {
    fps = entityCount = systems = 0;
    frameCount = 0;
}

static void init(Entity camera, Entity& fps, Entity& entityCount, Entity& systems) {
    const Vector2& cameraSize = TRANSFORM(camera)->size;

    fps = theEntityManager.CreateEntity("__debug_fps");
    ADD_COMPONENT(fps, Transformation);
    TRANSFORM(fps)->parent = camera;
    TRANSFORM(fps)->size = cameraSize * Vector2(0.3, 0.2);
    TRANSFORM(fps)->position = cameraSize * Vector2(-0.5, 0.5) + TRANSFORM(fps)->size * Vector2(0.5, -0.5);
    TRANSFORM(fps)->z = 0;
    ADD_COMPONENT(fps, Rendering);
    RENDERING(fps)->texture = theRenderingSystem.loadTextureFile(FpsTextureName);
    RENDERING(fps)->hide = false;
    ADD_COMPONENT(fps, Graph);
    GRAPH(fps)->textureName = FpsTextureName;
    GRAPH(fps)->minY = 0;
    GRAPH(fps)->maxY = 65;

    entityCount = theEntityManager.CreateEntity("__debug_entityCount");
    ADD_COMPONENT(entityCount, Transformation);
    TRANSFORM(entityCount)->parent = camera;
    TRANSFORM(entityCount)->size = cameraSize * Vector2(0.3, 0.2);
    TRANSFORM(entityCount)->position = cameraSize * Vector2(0, 0.5) + TRANSFORM(fps)->size * Vector2(0, -0.5);
    TRANSFORM(entityCount)->z = 0;
    ADD_COMPONENT(entityCount, Rendering);
    RENDERING(entityCount)->texture = theRenderingSystem.loadTextureFile(EntitiesTextureName);
    RENDERING(entityCount)->hide = false;
    ADD_COMPONENT(entityCount, Graph);
    GRAPH(entityCount)->textureName = EntitiesTextureName;

    systems = theEntityManager.CreateEntity("__debug_systems");
    ADD_COMPONENT(systems, Transformation);
    TRANSFORM(systems)->parent = camera;
    TRANSFORM(systems)->size = cameraSize * Vector2(0.3, 0.2);
    TRANSFORM(systems)->position = cameraSize * Vector2(0.5, 0.5) + TRANSFORM(fps)->size * Vector2(-0.5, -0.5);
    TRANSFORM(systems)->z = 0;
    ADD_COMPONENT(systems, Rendering);
    RENDERING(systems)->texture = theRenderingSystem.loadTextureFile(SystemsTextureName);
    RENDERING(systems)->hide = false;
}

static Entity createSystemGraphEntity(const std::string& name, Entity parent, int index) {
    Color color = Color::random();
    color.a = 1;
    Entity e = theEntityManager.CreateEntity(std::string("__debug_") + name);
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->parent = parent;
    TRANSFORM(e)->z = -0.002;
    TRANSFORM(e)->size = TRANSFORM(parent)->size;
    TRANSFORM(e)->position = TRANSFORM(parent)->size * (-0.5) - Vector2(0, (index + 1) * 1);

    ADD_COMPONENT(e, TextRendering);
    TEXT_RENDERING(e)->color = color;
    TEXT_RENDERING(e)->positioning = TextRenderingComponent::LEFT;
    TEXT_RENDERING(e)->flags = TextRenderingComponent::AdjustHeightToFillWidthBit;
    TEXT_RENDERING(e)->maxCharHeight = 0.5;
    TEXT_RENDERING(e)->text = name;

    ADD_COMPONENT(e, Graph);
    GRAPH(e)->textureName = SystemsTextureName;
    GRAPH(e)->minY = 0;
    GRAPH(e)->maxY = 0.020;
    GRAPH(e)->lineColor = color;

    return e;
}

void DebuggingSystem::DoUpdate(float dt) {
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
        init(activeCamera, fps, entityCount, systems); 
        LOG(INFO) << "Initialize DebugSystem: " << fps << ", " << entityCount << ", " << systems;
    }

    bool reloadTextures = (timeUntilGraphUpdate < 0);

    // Fill FPS graph
    GRAPH(fps)->pointsList.push_back(std::make_pair(frameCount, 1.0/dt));
    if (frameCount > 120) GRAPH(fps)->pointsList.pop_front();
    GRAPH(fps)->reloadTexture = reloadTextures;

    // Fill Entity count graph
    GRAPH(entityCount)->pointsList.push_back(std::make_pair(frameCount, theEntityManager.getNumberofEntity()));
    if (frameCount > 120) GRAPH(entityCount)->pointsList.pop_front();
    GRAPH(entityCount)->reloadTexture = reloadTextures;


    const std::vector<std::string> systemNames = ComponentSystem::registeredSystemNames();
    for (unsigned i=0; i<systemNames.size(); i++) {
        const ComponentSystem* system = ComponentSystem::Named(systemNames[i]);

        auto it = debugEntities.find(systemNames[i]);
        if (it == debugEntities.end()) {
            it = debugEntities.insert(std::make_pair(systemNames[i], createSystemGraphEntity(systemNames[i], systems, debugEntities.size()))).first;
        }
        Entity e = it->second;
        GRAPH(e)->pointsList.push_back(std::make_pair(frameCount, system->updateDuration));
        if (frameCount > 120) GRAPH(e)->pointsList.pop_front();
        GRAPH(e)->reloadTexture = reloadTextures;
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
        TRANSFORM(c)->size = Vector2(10, 1);
        TRANSFORM(c)->position = Vector2(0, -1);

        ADD_COMPONENT(c, TextRendering);
        TEXT_RENDERING(c)->fontName = "typo";
        TEXT_RENDERING(c)->hide = true;
        TEXT_RENDERING(c)->charHeight = 1;
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
#endif
}

#ifdef INGAME_EDITORS
void DebuggingSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    DebuggingComponent* dc = Get(entity, false);
    if (!dc) return;
}
#endif
