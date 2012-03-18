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
#else
#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>

#endif

#ifdef ANDROID
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
		LOGI("loadSound: '%s' (music:%d, env:%p)", assetName.c_str(), music, env);
		jstring asset = env->NewStringUTF(assetName.c_str());
		LOGI("jstring: %p", asset);
		LOGI("class: %p method: %p", javaSoundApi, jloadSound);
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
#else
#include <sstream>
struct OpenAlSoundAPI {
	ALuint loadSound(const std::string& Filename) {
		SF_INFO FileInfos;
		std::stringstream a;
		a << "assets/" << Filename;
		SNDFILE* File = sf_open(a.str().c_str(), SFM_READ, &FileInfos);
		if (!File) {
			LOGI("le fichier %s n'existe pas", Filename.c_str());
			return 0;
		}
		ALsizei NbSamples  = static_cast<ALsizei>(FileInfos.channels * FileInfos.frames);
		ALsizei SampleRate = static_cast<ALsizei>(FileInfos.samplerate);
		std::vector<ALshort> Samples(NbSamples);
		if (sf_read_short(File, &Samples[0], NbSamples) < NbSamples)
			return 0;
		sf_close(File);
		ALenum Format;
		switch (FileInfos.channels) {
			case 1 :  Format = AL_FORMAT_MONO16;   break;
			case 2 :  Format = AL_FORMAT_STEREO16; break;
			default : return 0;
		}
		ALuint Buffer;
		alGenBuffers(1, &Buffer);
		alBufferData(Buffer, Format, &Samples[0], NbSamples * sizeof(ALushort), SampleRate);
		if (alGetError() != AL_NO_ERROR)
			return 0;
		return Buffer;
	}
	ALuint play(ALuint soundId) {
		ALuint Source = 0;
		alGenSources(1, &Source);
		alSourcei(Source, AL_BUFFER, soundId);
		alSourcePlay(Source);
		return Source;
	}

	ALfloat musicPos(ALuint Source) {
	    ALfloat Seconds = 0.f;
	    alGetSourcef(Source, AL_SEC_OFFSET, &Seconds);
	    return Seconds;
	}

	void pauseAll() {
		//alSourcePause(it->second.source);
	}

	void resumeAll() {
		//alSourcePlay(it->second.source);
	}
};
#endif

struct SoundComponent {
	SoundComponent() : sound(InvalidSoundRef), position(0), started(false), source(0) {}
	SoundRef sound;
	enum { MUSIC, EFFECT } type;
	float position;
	bool repeat; /* si repeat est faux: qd le son a été joué en plein, on passe sound à InvalidSoundRef */
	/* openal specific datas : openAL source ? */
	bool started;
	#ifndef ANDROID
	ALuint source;
	#endif
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
std::map<SoundRef, ALuint> sounds;
#endif

public:
#ifdef ANDROID
	JavaSoundAPI* androidSoundAPI;
#else
	OpenAlSoundAPI* linuxSoundAPI;
#endif
};

