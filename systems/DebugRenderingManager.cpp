/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
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
		

