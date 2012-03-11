#include "SoundSystem.h"

INSTANCE_IMPL(SoundSystem);

SoundSystem::SoundSystem() : ComponentSystemImpl<SoundComponent>("sound"), nextValidRef(1) {
}

void SoundSystem::init() {

}

SoundRef SoundSystem::loadSoundFile(const std::string& assetName, bool music) {
#ifdef ANDROID
	if (!music && assetSounds.find(assetName) != assetSounds.end())
#else
	if (assetSounds.find(assetName) != assetSounds.end())
#endif
		return assetSounds[assetName];

#ifdef ANDROID
	int soundID = androidSoundAPI->loadSound(assetName, music);
	sounds[nextValidRef] = soundID;
#endif

	assetSounds[assetName] = nextValidRef;
#ifdef ANDROID
	
	LOGW("Sound : %s -> %d -> %d", assetName.c_str(), nextValidRef, soundID);
#endif

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
				#ifdef ANDROID
				androidSoundAPI->play (sounds[rc->sound], (rc->type == SoundComponent::MUSIC));
				#endif
				rc->started = true;
			} else if (rc->type == SoundComponent::MUSIC) {
				float newPos;
				#ifdef ANDROID
				newPos = androidSoundAPI->musicPos(sounds[rc->sound]);
				#endif
				if (newPos >= 0.999) {
					LOGW("sound ended (%d)", rc->sound);
					rc->position = 0;
					rc->sound = InvalidSoundRef;
					rc->started = false;
				} else {
					rc->position = newPos;
				}
			} else if (rc->type == SoundComponent::EFFECT) {
				rc->sound = InvalidSoundRef;
				rc->started = false;
			}
		}	
	}
}
