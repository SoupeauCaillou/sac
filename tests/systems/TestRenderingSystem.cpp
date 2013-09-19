/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <UnitTest++.h>

#include <base/Log.h>

#include <glm/glm.hpp>
#include "systems/RenderingSystem.h"
#include <algorithm>

TEST(save_restore_internalState)
{
LOGW("RESTORE TEST ?");
#if 0
	RenderingSystem::CreateInstance();
	std::map<std::string, TextureRef> initial;
	initial["plop"] = MathUtil::RandomInt(1000);
	initial["carnaval"] = MathUtil::RandomInt(1000);
	initial["youpi"] = MathUtil::RandomInt(1000);

	theRenderingSystem.assetTextures = initial;
	uint8_t* state;
	int size = theRenderingSystem.saveInternalState(&state);
	theRenderingSystem.assetTextures.clear();

	theRenderingSystem.restoreInternalState(state, size);

	CHECK_EQUAL(initial.size(), theRenderingSystem.assetTextures.size());
	for(std::map<std::string, TextureRef>::iterator it=initial.begin();
		it!=initial.end(); ++it) {
		std::map<std::string, TextureRef>::iterator jt = theRenderingSystem.assetTextures.find(it->first);
		CHECK(jt != theRenderingSystem.assetTextures.end());
		CHECK_EQUAL(it->second, jt->second);
	}
	RenderingSystem::DestroyInstance();
#endif
}
