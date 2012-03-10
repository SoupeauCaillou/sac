#include "SoundSystem.h"

INSTANCE_IMPL(SoundSystem);

SoundSystem::SoundSystem() : ComponentSystemImpl<SoundComponent>("sound"), nextValidRef(1) {
}

void SoundSystem::init() {
	/* preload sound effects */
}

SoundRef SoundSystem::loadSoundFile(const std::string& assetName, bool music) {
	if (assetSounds.find(assetName) != assetSounds.end())
		return assetSounds[assetName];

#ifdef ANDROID
	int soundID = androidSoundAPI->loadSound(assetName, music);
#endif
	sounds[nextValidRef] = soundID;
	assetSounds[assetName] = nextValidRef;
	
	LOGW("Sound : %s -> %d -> %d", assetName.c_str(), nextValidRef, soundID);

	return nextValidRef++;
}

void SoundSystem::DoUpdate(float dt) {
	/* play component with a valid sound ref */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;
		SoundComponent* rc = (*it).second;
		if (rc->sound != InvalidSoundRef) {
			if (!rc->started) {
				LOGW("sound started (%d)", rc->sound);
				androidSoundAPI->play (sounds[rc->sound], (rc->type == SoundComponent::MUSIC));
				rc->started = true;
			} else if (rc->type == SoundComponent::MUSIC) {
				float newPos = androidSoundAPI->musicPos(sounds[rc->sound]);
				if (newPos == rc->position && newPos >= 0.99) {
					LOGW("sound ended (%d)", rc->sound);
					rc->position = 0;
					rc->sound = InvalidSoundRef;
					rc->started = false;
				} else {
					rc->position = newPos;
				}
			}
		}	
	}
}
