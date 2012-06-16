/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
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

    datas->javaSoundApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/heriswap/HeriswapJNILib"));
    datas->jloadSound = jniMethodLookup(env, datas->javaSoundApi, "loadSound", "(Landroid/content/res/AssetManager;Ljava/lang/String;)I");
    datas->jplaySound = jniMethodLookup(env, datas->javaSoundApi, "playSound", "(IF)V");
    datas->initialized = true;
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
