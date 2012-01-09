#pragma once

#include <map>

class DebugRenderingManager {
	private:
		static DebugRenderingManager* _instance;
	public:
		static DebugRenderingManager& Instance() { 
			if (!_instance) _instance = new DebugRenderingManager();
			return *_instance; 
		}
		
		DebugRenderingManager();
		
		// void RegisterDebugRenderer(const String& msgName, Renderable* r);
		
		// void UnregisterDebugRenderer(const String& msgName);
		
		void Render();
		
	private:
		// std::map<String, Renderable*> enabledDebug;
		// std::map<String, Renderable*> disabledDebug;
};

#define theDebugRenderingManager DebugRenderingManager::Instance()
