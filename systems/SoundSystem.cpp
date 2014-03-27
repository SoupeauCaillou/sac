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



#include "SoundSystem.h"
#include "api/SoundAPI.h"

INSTANCE_IMPL(SoundSystem);

SoundSystem::SoundSystem() : ComponentSystemImpl<SoundComponent>("Sound"), nextValidRef(1), mute(false) {
    /* nothing saved */
    SoundComponent sc;

    componentSerializer.add(new Property<SoundRef>(HASH("sound", 0x6a44a74f), PropertyType::Sound, OFFSET(sound, sc), 0));
    componentSerializer.add(new Property<float>(HASH("volume", 0x5baaa86e), OFFSET(volume, sc), 0.001f));
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
            LOGE("Unable to load sound file: '" << assetName << "'");
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
        END_FOR_EACH()
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
    END_FOR_EACH()
}
