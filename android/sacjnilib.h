#pragma once

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
			env->DeleteGlobalRef(assetManager);
		}

		JNIEnvDependantContext::uninit(pEnv);
	}
};

struct AndroidGame {
	static AndroidGame* buildGame(GameThreadJNIEnvCtx* gameCtx);
	
};
