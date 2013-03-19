#include "DebuggingSystem.h"
#include "CameraSystem.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"
#include "GraphSystem.h" 

INSTANCE_IMPL(DebuggingSystem);

DebuggingSystem::DebuggingSystem() : ComponentSystemImpl<DebuggingComponent>("Debugging") {
	activeCamera = 0;
	
	fpsGraph = theEntityManager.CreateEntity("fpsGraph");
	ADD_COMPONENT(fpsGraph, Transformation);
	TRANSFORM(fpsGraph)->size = Vector2(10);
    TRANSFORM(fpsGraph)->position = Vector2(-10, 0);
//    TRANSFORM(fpsGraph)->z = 1;
	ADD_COMPONENT(fpsGraph, Rendering);
	RENDERING(fpsGraph)->hide = false;
    RENDERING(fpsGraph)->texture = theRenderingSystem.loadTextureFile("fpsGraph");
    ADD_COMPONENT(fpsGraph, Graph);
    GRAPH(fpsGraph)->textureName = "fpsGraph";
    GRAPH(fpsGraph)->maxY = 70;
    GRAPH(fpsGraph)->minY = 10;
	
	entityGraph = theEntityManager.CreateEntity("entityGraph");
	ADD_COMPONENT(entityGraph, Transformation);
	TRANSFORM(entityGraph)->size = Vector2(10);
    TRANSFORM(entityGraph)->position = Vector2(10, 0);
    // TRANSFORM(entityGraph)->z = 1;
	ADD_COMPONENT(entityGraph, Rendering);
	RENDERING(entityGraph)->hide = false;
    RENDERING(entityGraph)->texture = theRenderingSystem.loadTextureFile("entityGraph");
	ADD_COMPONENT(entityGraph, Graph);
	GRAPH(entityGraph)->textureName = "entityGraph";
    GRAPH(entityGraph)->maxY = 0;
    GRAPH(entityGraph)->minY = 70;
    GRAPH(entityGraph)->setFixedScaleMinMaxY = true;
    
	
	for (int i=0; i<17; ++i) {
		timeSpentinSystemGraph[i] = theEntityManager.CreateEntity("timeSpentinSystemGraph");
		ADD_COMPONENT(timeSpentinSystemGraph[i], Transformation);
		TRANSFORM(timeSpentinSystemGraph[i])->size = Vector2(10);
	    TRANSFORM(timeSpentinSystemGraph[i])->position = Vector2::Zero;
//	    TRANSFORM(timeSpentinSystemGraph[i])->z = 1;
		ADD_COMPONENT(timeSpentinSystemGraph[i], Rendering);
		RENDERING(timeSpentinSystemGraph[i])->hide = i == 0 ? false : true;
	    RENDERING(timeSpentinSystemGraph[i])->texture = theRenderingSystem.loadTextureFile("timeSpentinSystemGraph");
		ADD_COMPONENT(timeSpentinSystemGraph[i], Graph);
		GRAPH(timeSpentinSystemGraph[i])->textureName = "timeSpentinSystemGraph";
	    GRAPH(timeSpentinSystemGraph[i])->maxY = 0.020f;
	    GRAPH(timeSpentinSystemGraph[i])->minY = 0;
	    GRAPH(timeSpentinSystemGraph[i])->lineColor = Color::random();
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
		
		if (activeCamera) {
			TRANSFORM(fpsGraph)->parent = activeCamera;
			TRANSFORM(entityGraph)->parent = activeCamera;
			for (int i=0; i<17; ++i) {
				TRANSFORM(timeSpentinSystemGraph[i])->parent = activeCamera;
			}
		}
	}

	while(GRAPH(fpsGraph)->pointsList.size() > 150) GRAPH(fpsGraph)->pointsList.pop_front();
	
	while(GRAPH(entityGraph)->pointsList.size() > 150) GRAPH(entityGraph)->pointsList.pop_front();
	
	for (int i=0; i<17; ++i) {
		while(GRAPH(timeSpentinSystemGraph[i])->pointsList.size() > 60) GRAPH(timeSpentinSystemGraph[i])->pointsList.pop_front();
	}
	
	static float reloadFrequency = 0;
	reloadFrequency += dt;
	if (reloadFrequency > 0.5f) {
		reloadFrequency = 0;
		GRAPH(fpsGraph)->reloadTexture = true;
		GRAPH(entityGraph)->reloadTexture = true;
		GRAPH(timeSpentinSystemGraph[0])->reloadTexture= true;
	}
	
}


void DebuggingSystem::addValue(DEBUGGINGENTITY entity, std::pair<float, float> value) {
	if (entity == fpsGraphEntity)
		GRAPH(fpsGraph)->pointsList.push_back(value);
			
	else if (entity == entityGraphEntity) 
			GRAPH(entityGraph)->pointsList.push_back(value);
	else
			GRAPH(timeSpentinSystemGraph[entity - 2])->pointsList.push_back(value);
}

void DebuggingSystem::clearDebuggingEntity(DEBUGGINGENTITY entity) {
	switch(entity) {
		case fpsGraphEntity:
			GRAPH(fpsGraph)->pointsList.clear();
			break;
		case entityGraphEntity:
			GRAPH(entityGraph)->pointsList.clear();
			break;
		default:
			GRAPH(timeSpentinSystemGraph[entity - 2])->pointsList.clear();
	}
}

#ifdef INGAME_EDITORS
void DebuggingSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    DebuggingComponent* tc = Get(entity, false);
    if (!tc) return;
}
#endif

