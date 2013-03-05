#include <UnitTest++.h>

#include "base/MathUtil.h"
#include "systems/RenderingSystem.h"
#include <algorithm>

TEST(save_restore_internalState)
{
LOG(WARNING) << "RESTORE TEST ?";
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
