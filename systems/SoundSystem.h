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

typedef int SoundRef;

#define InvalidSoundRef -1

#ifdef ANDROID
#include <jni.h>
#else
#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>

#endif

#define MUSIC_VISU 

#ifdef ANDROID
struct JavaSoundAPI {
	JNIEnv* env;
	jobject assetManager;
	jclass javaSoundApi;
	jmethodID jloadSound;
	jmethodID jplaySound;
    jmethodID jstopSound;
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

	int play(int soundId, bool music, int soundMaster, int offset_ms) {
		return env->CallStaticIntMethod(javaSoundApi, jplaySound, soundId, music, soundMaster, offset_ms);
	}

    void stop(int soundId, bool music) {
        env->CallStaticVoidMethod(javaSoundApi, jstopSound, soundId, music);
    }

	float musicPos(int soundId) {
		return env->CallStaticFloatMethod(javaSoundApi, jmusicPos, soundId);
	}

	void pauseAll() {
		env->CallStaticVoidMethod(javaSoundApi, jpauseSounds);
	}

	void resumeAll() {
		env->CallStaticVoidMethod(javaSoundApi, jresumeSounds);
	}
};
#else
#include <sstream>
#include <cassert>

#define AL_OPERATION(x)	\
		(x); \
		OpenAlSoundAPI::check_AL_errors(#x);
		
struct OpenAlSoundAPI {
		
	static void check_AL_errors(const char* context) {
		int maxIterations=10;
    	ALenum error;
    	bool err = false;
    	while (((error = alGetError()) != AL_NO_ERROR) && maxIterations > 0) {
	    	LOGW("OpenAL error during '%s' -> %s", context, errToString(error));
			maxIterations--;
			err = true;
    	}
    	assert(!err);
    }
		
	static const char* errToString(ALenum err) {
		switch (err) {
			case AL_NO_ERROR: return "AL(No error)";
			case AL_INVALID_NAME: return "AL(Invalid name)";
			case AL_INVALID_VALUE: return "AL(Invalid value)";
			case AL_INVALID_ENUM: return "AL(Invalid enum)";
			case AL_INVALID_OPERATION: return "AL(Invalid operation)";
			case AL_OUT_OF_MEMORY: return "AL(Out of memory)";
			default: return "AL(Unknown)";
		}
	}

	/* return an OpenAL buffer */
	bool loadSound(const std::string& Filename, ALuint* out) {
		SF_INFO FileInfos;
		std::stringstream a;
		a << "assets/" << Filename;
		SNDFILE* File = sf_open(a.str().c_str(), SFM_READ, &FileInfos);
		if (!File) {
			LOGI("le fichier %s n'existe pas", Filename.c_str());
			return false;
		}
		ALsizei NbSamples  = static_cast<ALsizei>(FileInfos.channels * FileInfos.frames);
		ALsizei SampleRate = static_cast<ALsizei>(FileInfos.samplerate);
		std::vector<ALshort> Samples(NbSamples);
		if (sf_read_short(File, &Samples[0], NbSamples) < NbSamples) {
			LOGI("invalid sound file (%s)", Filename.c_str());
			return false;
		}
		sf_close(File);
		ALenum Format;
		switch (FileInfos.channels) {
			case 1 :  Format = AL_FORMAT_MONO16;   break;
			case 2 :  Format = AL_FORMAT_STEREO16; break;
			default : LOGW("Invalid format"); return false;
		}
		ALuint Buffer;
		AL_OPERATION(alGenBuffers(1, &Buffer))
		AL_OPERATION(alBufferData(Buffer, Format, &Samples[0], NbSamples * sizeof(ALushort), SampleRate))
		*out = Buffer;
		return true;
	}
	/* return the OpenAL source used to play the sound */
	ALuint play(ALuint buffer, ALuint source, ALfloat seek) {
		// LOGW("PLAY %d, %d, %.2f", buffer, source, seek);
		int state;
		AL_OPERATION(alGetSourcei(source, AL_SOURCE_STATE, &state))
		assert (state != AL_PLAYING && state != AL_PAUSED); 
		AL_OPERATION(alSourcei(source, AL_BUFFER, buffer))
		moveToSeek(source, buffer, seek); // on joue la musique à partir de seek € [0,1]
		AL_OPERATION(alSourcePlay(source))
		return source;
	}

	ALfloat musicPos(ALuint Source, ALuint buffer) {
		ALfloat pos = 0;
		ALint tot = 0;
		AL_OPERATION(alGetSourcef(Source, AL_BYTE_OFFSET, &pos))
		AL_OPERATION(alGetBufferi(buffer, AL_SIZE, &tot))

		return pos/(float)tot;
	}

	void moveToSeek(ALuint source, ALuint buffer, ALfloat seek) {
		//seek in [0,1]
		ALint tot = 0;
		AL_OPERATION(alGetBufferi(buffer, AL_SIZE, &tot))
		AL_OPERATION(alSourcei(source, AL_BYTE_OFFSET, seek*tot))
	}
};
#endif

struct SoundComponent {
	SoundComponent() : sound(InvalidSoundRef), masterTrack(0), position(0), started(false), seek(0), stop(false), repeat(false), fadeOut(0.7) {
		#ifndef ANDROID
		source = 0;
		#endif
	}
	SoundRef sound;
	SoundComponent* masterTrack;
	float masterTrackOffsetMs;
	enum { MUSIC, EFFECT } type;
	float position; // in [0,1]
	bool repeat; /* si repeat est faux: qd le son a été joué entiérement, on passe sound à InvalidSoundRef */
	bool started, stop;
	float seek;
	float fadeOut; // durée en sec
	float volume;
	#ifndef ANDROID
	ALuint source; // openAL source
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
// SoundRef -> OpenAL buffer mapping
std::map<SoundRef, ALuint> sounds;
std::list<ALuint> availableSources;
std::list<ALuint> activeSources;
#endif

#ifdef MUSIC_VISU
std::map<Entity, Entity> visualisationEntities;
#endif

public:
#ifdef ANDROID
	JavaSoundAPI* androidSoundAPI;
#else
	OpenAlSoundAPI* linuxSoundAPI;
#endif
bool mute;

};

