#include "SoundSystem.h"

INSTANCE_IMPL(SoundSystem);

SoundSystem::SoundSystem() : ComponentSystemImpl<SoundComponent>("sound") {
}

void SoundSystem::init() {

}

SoundRef SoundSystem::loadSoundFile(const std::string& assetName) {
	if (assetSounds.find(assetName) != assetSounds.end())
		return assetSounds[assetName];

	if (!assetLoader)
		return 0;

	// TODO

	//sounds[nextValidRef] = texture;
	assetSounds[assetName] = nextValidRef;

	return nextValidRef++;
}



void SoundSystem::DoUpdate(float dt) {
	/* play component with a valid sound ref */
}
