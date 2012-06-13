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
