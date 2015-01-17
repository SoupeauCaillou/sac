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



#pragma once

#include <cstdio>
#include <cstdlib>
#include <list>
#include <algorithm>

#include "System.h"

class SoundAPI;
struct OpaqueSoundPtr;
typedef int SoundRef;

#define InvalidSoundRef -1

struct SoundComponent {
        SoundComponent() : sound(InvalidSoundRef), volume(1) { }
        SoundRef sound;
        float volume;
};

#define theSoundSystem SoundSystem::GetInstance()
#if SAC_DEBUG
#define SOUND(e) theSoundSystem.Get(e,true,__FILE__,__LINE__)
#else
#define SOUND(e) theSoundSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Sound)

public:
~SoundSystem();
void init();
SoundRef loadSoundFile(const std::string& assetName);

private:
/* textures cache */
SoundRef nextValidRef;
std::map<std::string, SoundRef> assetSounds;
std::map<SoundRef, OpaqueSoundPtr*> sounds;

public:
SoundAPI* soundAPI;
bool mute;

};

