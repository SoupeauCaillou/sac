#pragma once

#include "System.h"

struct DebuggingComponent {
};

#define theDebuggingSystem DebuggingSystem::GetInstance()
#define DEBUGGING(e) theDebuggingSystem.Get(e)

UPDATABLE_SYSTEM(Debugging)

	public:
		enum DEBUGGINGENTITY {
			fpsGraphEntity=0,
			entityGraphEntity,
			TransformationSystemEntity,
			RenderingSystemEntity,
			SoundSystemEntity,
    		MusicSystemEntity,
			ADSRSystemEntity,
			ButtonSystemEntity,
			TextRenderingSystemEntity,
			ContainerSystemEntity,
			PhysicsSystemEntity,
    		ParticuleSystemEntity,
    		ScrollingSystemEntity,
    		MorphingSystemEntity,
    		AutonomousAgentSystemEntity,
    		AnimationSystemEntity,
    		AutoDestroySystemEntity,
    		CameraSystemEntity,
    		GraphSystemEntity
		};
		
		void addValue(DEBUGGINGENTITY entity, std::pair<float, float> value);
		void clearDebuggingEntity(DEBUGGINGENTITY entity);

	private:
		Entity fpsGraph;
		Entity entityGraph;
		Entity timeSpentinSystemGraph[17];
		
		Entity activeCamera;
};

