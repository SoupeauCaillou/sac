#include "SoundSystem.h"


INSTANCE_IMPL(SoundSystem);

SoundSystem::SoundSystem() : ComponentSystemImpl<SoundComponent>("Sound"), nextValidRef(1), mute(false) {
    /* nothing saved */
}

SoundSystem::~SoundSystem() {
    for (std::map<SoundRef, OpaqueSoundPtr*>::iterator it=sounds.begin(); it!=sounds.end(); ++it) {
        delete it->second;
    }
    sounds.clear();
}

void SoundSystem::init() {
	nextValidRef = 1;
	assetSounds.clear();
	for (std::map<SoundRef, OpaqueSoundPtr*>::iterator it=sounds.begin(); it!=sounds.end(); ++it) {
		delete it->second;
	}
	sounds.clear();
}

SoundRef SoundSystem::loadSoundFile(const std::string& assetName) {
	PROFILE("Sound", "loadSoundFile", BeginEvent);
    if (assetSounds.find(assetName) != assetSounds.end()) {
	    PROFILE("Sound", "loadSoundFile", EndEvent);
        return assetSounds[assetName];
    } else {
        OpaqueSoundPtr* ptr = soundAPI->loadSound(assetName);
        if (!ptr) {
            LOG(ERROR) << "Unable to load sound file: '" << assetName << "'";
            PROFILE("Sound", "loadSoundFile", EndEvent);
            return InvalidSoundRef;
        } else {
            sounds[nextValidRef] = ptr;
            assetSounds[assetName] = nextValidRef;
            PROFILE("Sound", "loadSoundFile", EndEvent);
            return nextValidRef++;
        }
    }
}

void SoundSystem::DoUpdate(float) {
	if (mute) {
		FOR_EACH_COMPONENT(Sound, rc)
            rc->sound = InvalidSoundRef;
        }
        return;
    }

	/* play component with a valid sound ref */
    FOR_EACH_COMPONENT(Sound, rc)
		if (rc->sound != InvalidSoundRef && !mute ) {
			std::map<SoundRef, OpaqueSoundPtr*>::iterator jt = sounds.find(rc->sound);
			if (jt != sounds.end()) {
				// TODO: declare N max tries, then stop trying to play the sound
				if (soundAPI->play(sounds[rc->sound], rc->volume) || true) {
            		rc->sound = InvalidSoundRef;
				}
			} else {
				rc->sound = InvalidSoundRef;
			}
        }
    }
}

#ifdef INGAME_EDITORS
void SoundSystem::addEntityPropertiesToBar(Entity e, TwBar* bar) {

}
#endif
