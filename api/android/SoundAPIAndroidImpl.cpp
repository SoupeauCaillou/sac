#include "SoundAPIAndroidImpl.h"
#include "base/Log.h"

struct AndroidSoundOpaquePtr : public OpaqueSoundPtr {
    jint soundID;
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

SoundAPIAndroidImpl::~SoundAPIAndroidImpl() {
    env->DeleteGlobalRef(datas->javaSoundApi);
    delete datas;
}

void SoundAPIAndroidImpl::init() {
    datas = new SoundAPIAndroidImplData();

    datas->javaSoundApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib"));
    datas->jloadSound = jniMethodLookup(env, datas->javaSoundApi, "loadSound", "(Landroid/content/res/AssetManager;Ljava/lang/String;)I");
    datas->jplaySound = jniMethodLookup(env, datas->javaSoundApi, "playSound", "(IF)V");
}

OpaqueSoundPtr* SoundAPIAndroidImpl::loadSound(const std::string& asset) {
    LOGI("loadSound: '%s'", asset.c_str());
    jstring jasset = env->NewStringUTF(asset.c_str());
    AndroidSoundOpaquePtr* out = new AndroidSoundOpaquePtr();
    out->soundID = env->CallStaticIntMethod(datas->javaSoundApi, datas->jloadSound, assetManager, jasset);
    return out;
}

void SoundAPIAndroidImpl::play(OpaqueSoundPtr* p, float volume) {
    AndroidSoundOpaquePtr* ptr = static_cast<AndroidSoundOpaquePtr*>(p);
    env->CallStaticVoidMethod(datas->javaSoundApi, datas->jplaySound, ptr->soundID, volume);
}
