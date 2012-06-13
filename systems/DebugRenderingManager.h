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
