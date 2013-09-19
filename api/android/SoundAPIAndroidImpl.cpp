/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



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
