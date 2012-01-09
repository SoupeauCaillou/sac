#include "DebugRenderingManager.h"

DebugRenderingManager* DebugRenderingManager::_instance = 0;

DebugRenderingManager::DebugRenderingManager() {
	_instance = this;
}

#if 0
void DebugRenderingManager::RegisterDebugRenderer(const String& msgName, Renderable* r) {
	theSwitchboard.SubscribeTo(this, msgName);
	disabledDebug[msgName] = r;
}

void DebugRenderingManager::UnregisterDebugRenderer(const String& msgName) {
	theSwitchboard.UnsubscribeFrom(this, msgName);
	disabledDebug[msgName] = 0;
	enabledDebug[msgName] = 0;
}
#endif
		
void DebugRenderingManager::Render() {
/*
	for(std::map<String, Renderable*>::iterator it=enabledDebug.begin();
		it != enabledDebug.end(); ++it) {
			Renderable* r = (*it).second;
			if (r) 
				r->Render();	
	}
*/
}
		

