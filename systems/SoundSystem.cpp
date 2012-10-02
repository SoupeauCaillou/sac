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
#include "SoundSystem.h"
#ifdef MUSIC_VISU
#include "RenderingSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"
#endif

INSTANCE_IMPL(SoundSystem);

SoundSystem::SoundSystem() : ComponentSystemImpl<SoundComponent>("Sound"), nextValidRef(1), mute(false) {
    /* nothing saved */
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
            LOGW("Unable to load sound file: '%s'", assetName.c_str());
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
