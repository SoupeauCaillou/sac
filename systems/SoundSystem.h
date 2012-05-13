#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <algorithm>
#include <map>

#include "base/Vector2.h"
#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include "api/SoundAPI.h"

typedef int SoundRef;

#define InvalidSoundRef -1

struct SoundComponent {
	SoundComponent() : sound(InvalidSoundRef), volume(1) { }
	SoundRef sound;
	float volume;
};

#define theSoundSystem SoundSystem::GetInstance()
#define SOUND(e) theSoundSystem.Get(e)

UPDATABLE_SYSTEM(Sound)

public:
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

