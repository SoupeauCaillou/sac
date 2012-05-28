#include "SoundSystem.h"
#ifdef MUSIC_VISU
#include "RenderingSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"
#endif

INSTANCE_IMPL(SoundSystem);

SoundSystem::SoundSystem() : ComponentSystemImpl<SoundComponent>("Sound"), nextValidRef(1), mute(false) {
}

void SoundSystem::init() {
    soundAPI->init();
}

SoundRef SoundSystem::loadSoundFile(const std::string& assetName) {
    if (assetSounds.find(assetName) != assetSounds.end()) {
        return assetSounds[assetName];
    } else {
        OpaqueSoundPtr* ptr = soundAPI->loadSound(assetName);
        if (!ptr) {
            LOGW("Unable to load sound file: '%s'", assetName.c_str());
            return InvalidSoundRef;
        } else {
            sounds[nextValidRef] = ptr;
            assetSounds[assetName] = nextValidRef;
            return nextValidRef++;
        }
    }
}

void SoundSystem::DoUpdate(float dt __attribute__((unused))) {
	if (mute) {
		for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
			SoundComponent* rc = (*it).second;
            rc->sound = InvalidSoundRef;
        }
        return;
    }

	/* play component with a valid sound ref */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		SoundComponent* rc = (*it).second;
		if (rc->sound != InvalidSoundRef && !mute ) {
            soundAPI->play(sounds[rc->sound], rc->volume);
            rc->sound = InvalidSoundRef;
        }
    }
}
