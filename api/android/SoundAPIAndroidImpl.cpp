#include "SoundAPIAndroidImpl.h"
#include "base/Log.h"

struct AndroidSoundOpaquePtr : public OpaqueSoundPtr {
    jint soundID;
};
struct SoundAPIAndroidImpl::SoundAPIAndroidImplData {
    jclass javaSoundApi;
    jmethodID jloadSound;
    jmethodID jplaySound;
    bool initialized;
};

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}


SoundAPIAndroidImpl::SoundAPIAndroidImpl() {
	datas = new SoundAPIAndroidImplData();
	datas->initialized = false;
}

SoundAPIAndroidImpl::~SoundAPIAndroidImpl() {
    uninit();
    delete datas;
}

void SoundAPIAndroidImpl::uninit() {
	if (datas->initialized) {
		env->DeleteGlobalRef(datas->javaSoundApi);
		datas->initialized = false;
	}
}

void SoundAPIAndroidImpl::init(JNIEnv *pEnv, jobject assetMgr) {
	env = pEnv;
	assetManager = assetMgr;

    datas->javaSoundApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/SoundAPI"));
    datas->jloadSound = jniMethodLookup(env, datas->javaSoundApi, "loadSound", "(Landroid/content/res/AssetManager;Ljava/lang/String;)I");
    datas->jplaySound = jniMethodLookup(env, datas->javaSoundApi, "playSound", "(IF)Z");
    datas->initialized = true;
}

OpaqueSoundPtr* SoundAPIAndroidImpl::loadSound(const std::string& asset) {
    LOGI("loadSound: '%s'", asset.c_str());
    jstring jasset = env->NewStringUTF(asset.c_str());
    AndroidSoundOpaquePtr* out = new AndroidSoundOpaquePtr();
    out->soundID = env->CallStaticIntMethod(datas->javaSoundApi, datas->jloadSound, assetManager, jasset);
    return out;
}

bool SoundAPIAndroidImpl::play(OpaqueSoundPtr* p, float volume) {
    AndroidSoundOpaquePtr* ptr = static_cast<AndroidSoundOpaquePtr*>(p);
    return env->CallStaticBooleanMethod(datas->javaSoundApi, datas->jplaySound, ptr->soundID, volume);
}
