#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <map>

#include "base/Vector2.h"
#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"

typedef int SoundRef;

class NativeSoundAssetLoader {
	public:
		virtual char* decompressOggSound(const std::string& assetName) = 0;
};

struct SoundComponent {
	SoundRef sound;
	float position;
	/* openal specific datas : openAL source ? */
};

#define theSoundSystem SoundSystem::GetInstance()
#define SOUND(e) theSoundSystem.Get(e)

UPDATABLE_SYSTEM(Sound)

public:
void init();

SoundRef loadSoundFile(const std::string& assetName);

void setNativeSoundAssetLoader(NativeSoundAssetLoader* ptr) { assetLoader = ptr; }

private:
/* textures cache */
SoundRef nextValidRef;
std::map<std::string, SoundRef> assetSounds;
#ifdef ANDROID
std::map<SoundRef, int/* mettre le bon type ici */> sounds;
#else
std::map<SoundRef, int/* mettre le bon type ici */> sounds;
#endif

NativeSoundAssetLoader* assetLoader;
};
