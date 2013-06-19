#include "SoundAPIAndroidImpl.h"

struct AndroidSoundOpaquePtr : public OpaqueSoundPtr {
    jint soundID;
};

SoundAPIAndroidImpl::SoundAPIAndroidImpl() : JNIWrapper<jni_sound_api::Enum>("net/damsy/soupeaucaillou/api/SoundAPI", true) {
	declareMethod(jni_sound_api::LoadSound, "loadSound", "(Ljava/lang/String;)I");
    declareMethod(jni_sound_api::Play, "playSound", "(IF)Z");
}

OpaqueSoundPtr* SoundAPIAndroidImpl::loadSound(const std::string& asset) {
    LOGI("loadSound: '" << asset << "'");
    jstring jasset = env->NewStringUTF(asset.c_str());
    AndroidSoundOpaquePtr* out = new AndroidSoundOpaquePtr();
    out->soundID = env->CallIntMethod(instance, methods[jni_sound_api::LoadSound], jasset);
    return out;
}

bool SoundAPIAndroidImpl::play(OpaqueSoundPtr* p, float volume) {
    AndroidSoundOpaquePtr* ptr = static_cast<AndroidSoundOpaquePtr*>(p);
    return env->CallBooleanMethod(instance, methods[jni_sound_api::Play], ptr->soundID, volume);
}
