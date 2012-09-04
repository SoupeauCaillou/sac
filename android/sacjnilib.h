#pragma once
#include <jni.h>

#include "api/android/AssetAPIAndroidImpl.h"
#include "api/android/MusicAPIAndroidImpl.h"
#include "api/android/SoundAPIAndroidImpl.h"
#include "api/android/LocalizeAPIAndroidImpl.h"
#include "api/android/NameInputAPIAndroidImpl.h"
#include "api/android/AdAPIAndroidImpl.h"
#include "api/android/ExitAPIAndroidImpl.h"
#include "api/android/SuccessAPIAndroidImpl.h"

struct JNIEnvDependantContext {
	virtual void init(JNIEnv* pEnv, jobject assetMgr) { env = pEnv; }
	virtual void uninit(JNIEnv* pEnv) { env = 0; }

	JNIEnv* env;
};

struct GameThreadJNIEnvCtx : JNIEnvDependantContext {
	NameInputAPIAndroidImpl nameInput;
	LocalizeAPIAndroidImpl localize;
    AdAPIAndroidImpl ad;
    AssetAPIAndroidImpl asset;
	ExitAPIAndroidImpl exitAPI;
	MusicAPIAndroidImpl musicAPI;
	SoundAPIAndroidImpl soundAPI;
	SuccessAPIAndroidImpl successAPI;
	jobject assetManager;

	virtual void init(JNIEnv* env, jobject assetMgr) {
		assetManager = env->NewGlobalRef(assetMgr);

		nameInput.init(env);
		localize.init(env);
	    ad.init(env);
	    asset.init(env, assetManager);
		exitAPI.init(env);
		musicAPI.init(env);
		soundAPI.init(env, assetManager);
		successAPI.init(env);

		JNIEnvDependantContext::init(env, assetMgr);
	}

	virtual void uninit(JNIEnv* pEnv) {
		if (env == pEnv) {
			nameInput.uninit();
			localize.uninit();
		    ad.uninit();
		    asset.uninit();
			exitAPI.uninit();
			musicAPI.uninit();
			soundAPI.uninit();
			successAPI.uninit();
			env->DeleteGlobalRef(assetManager);
		}

		JNIEnvDependantContext::uninit(pEnv);
	}
};

struct RenderThreadJNIEnvCtx : JNIEnvDependantContext {
    AssetAPIAndroidImpl asset;
	jobject assetManager;

	void init(JNIEnv* env, jobject assetMgr) {
		assetManager = env->NewGlobalRef(assetMgr);
	    asset.init(env, assetManager);
		JNIEnvDependantContext::init(env, assetMgr);
	}

	void uninit(JNIEnv* pEnv) {
		if (env == pEnv) {
			asset.uninit();
			env->DeleteGlobalRef(assetManager);
		}
		JNIEnvDependantContext::uninit(pEnv);
	}
};

struct Game;

class GameHolder {
	public:
	static GameHolder* build();
	
	Game* game;
	int width, height;

	GameThreadJNIEnvCtx* gameThreadJNICtx;
	RenderThreadJNIEnvCtx renderThreadJNICtx;

	struct __input {
		 int touching;
		 float x, y;
	} input;
	bool firstCall;
	struct timeval startup_time;
	float dtAccumuled, time;

	int openGLESVersion;
	bool initDone;
	
	private:
	GameHolder() {}
};