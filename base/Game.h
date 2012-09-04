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

#include <stdint.h>
#include <string>
class AssetAPI;

class Game {
	public:
		Game();

		virtual ~Game();
		
		void sacInit(int windowW, int windowH);
		
		virtual void init(const uint8_t* in = 0, int size = 0) = 0;
		
		virtual void tick(float dt) = 0;
		
		virtual void backPressed();
		
		virtual void togglePause(bool activate);
		
		virtual int saveState(uint8_t** out);
		
	protected:
		void loadFont(AssetAPI* asset, const std::string& name);
};