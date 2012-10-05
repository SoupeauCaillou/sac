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

	virtual void init(JNIEnv* pEnv, jobject assetMgr) {
		LOGW("%p", pEnv);
		assetManager = pEnv->NewGlobalRef(assetMgr);

		nameInput.init(pEnv);
		localize.init(pEnv);
	    ad.init(pEnv);
	    asset.init(pEnv, assetManager);
		exitAPI.init(pEnv);
		musicAPI.init(pEnv);
		soundAPI.init(pEnv, assetManager);
		successAPI.init(pEnv);

		JNIEnvDependantContext::init(pEnv, assetMgr);
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

	void init(JNIEnv* pEnv, jobject assetMgr) {
		LOGW("%p", pEnv);
		assetManager = pEnv->NewGlobalRef(assetMgr);
	    asset.init(pEnv, assetManager);
		JNIEnvDependantContext::init(pEnv, assetMgr);
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