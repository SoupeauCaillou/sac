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

#define InvalidSoundRef -1

#ifdef ANDROID
#include <jni.h>
struct JavaSoundAPI {
	JNIEnv* env;
	jobject assetManager;
	jclass javaSoundApi;
	jmethodID jloadSound;
	jmethodID jplaySound;
	jmethodID jmusicPos;
	jmethodID jpauseSounds;
	jmethodID jresumeSounds;
	
	int loadSound(const std::string& assetName, bool music) {
		jstring asset = env->NewStringUTF(assetName.c_str());
		int result = env->CallStaticIntMethod(javaSoundApi, jloadSound, assetManager, asset, music);
		return result;
	}
	
	int play(int soundId, bool music) {
		return env->CallStaticIntMethod(javaSoundApi, jplaySound, soundId, music);
	}
	
	float musicPos(int soundId) {
		return env->CallStaticFloatMethod(javaSoundApi, jmusicPos, soundId);
	}
	
	void pauseAll() {
		env->CallStaticObjectMethod(javaSoundApi, jpauseSounds);
	}
	
	void resumeAll() {
		env->CallStaticObjectMethod(javaSoundApi, jresumeSounds);
	}
};

#endif

struct SoundComponent {
	SoundComponent() : sound(InvalidSoundRef), position(0), started(false) {}
	SoundRef sound;
	enum { MUSIC, EFFECT } type;
	float position;
	bool repeat; /* si repeat est faux: qd le son a été joué en plein, on passe sound à InvalidSoundRef */
	/* openal specific datas : openAL source ? */
	bool started;
};

#define theSoundSystem SoundSystem::GetInstance()
#define SOUND(e) theSoundSystem.Get(e)

UPDATABLE_SYSTEM(Sound)

public:
void init();

SoundRef loadSoundFile(const std::string& assetName, bool music);

private:
/* textures cache */
SoundRef nextValidRef;
std::map<std::string, SoundRef> assetSounds;
#ifdef ANDROID
std::map<SoundRef, int> sounds;
#else
std::map<SoundRef, int/* mettre le bon type ici */> sounds;
#endif

#ifdef ANDROID
public:
	JavaSoundAPI* androidSoundAPI;
#endif
};

