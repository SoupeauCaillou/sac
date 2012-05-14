#include "SoundAPIAndroidImpl.h"
#include "base/Log.h"

struct AndroidSoundOpaquePtr : public OpaqueSoundPtr {
    jobject player;
};
struct SoundAPIAndroidImpl::SoundAPIAndroidImplData {
    jclass javaSoundApi;
    jmethodID jloadSound;
    jmethodID jplaySound;
};

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}


SoundAPIAndroidImpl::SoundAPIAndroidImpl(JNIEnv *pEnv, jobject assetMgr) : env(pEnv), assetManager(assetMgr) {

}

void SoundAPIAndroidImpl::init() {
    datas = new SoundAPIAndroidImplData();

    datas->javaSoundApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib"));
    datas->jloadSound = jniMethodLookup(env, datas->javaSoundApi, "createPlayer", "(I)Ljava/lang/Object;");
    datas->jplaySound = jniMethodLookup(env, datas->javaSoundApi, "pcmBufferSize", "(I)I");
}

OpaqueSoundPtr* SoundAPIAndroidImpl::loadSound(const std::string& asset) {
    LOGI("loadSound: '%s'", asset.c_str());
    jstring jasset = env->NewStringUTF(asset.c_str());
    jobject player = env->CallStaticObjectMethod(datas->javaSoundApi, datas->jloadSound, assetManager, jasset);
    AndroidSoundOpaquePtr* out = new AndroidSoundOpaquePtr();
    out->player = (jobject)env->NewGlobalRef(player);
    return out;
}

void SoundAPIAndroidImpl::play(OpaqueSoundPtr* p, float volume) {
    AndroidSoundOpaquePtr* ptr = static_cast<AndroidSoundOpaquePtr*>(p);
    return env->CallStaticVoidMethod(datas->javaSoundApi, datas->jplaySound, ptr->player, volume);
}
