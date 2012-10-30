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
#include "VibrateAPIAndroidImpl.h"
#include "base/Log.h"
#include <string>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}

struct VibrateAPIAndroidImpl::VibrateAPIAndroidImplData {
 jclass cls;
 jmethodID vibrate;
 bool initialized;
};

VibrateAPIAndroidImpl::VibrateAPIAndroidImpl() {
 datas = new VibrateAPIAndroidImplData();
 datas->initialized = false;
}

VibrateAPIAndroidImpl::~VibrateAPIAndroidImpl() {
 uninit();
    delete datas;
}

void VibrateAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;

    datas->cls = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/VibrateAPI"));
    datas->vibrate = jniMethodLookup(env, datas->cls, "vibrate", "(F)V");

    datas->initialized = true;
}

void VibrateAPIAndroidImpl::uninit() {
 if (datas->initialized) {
     env->DeleteGlobalRef(datas->cls);
     datas->initialized = false;
 }
}

void VibrateAPIAndroidImpl::vibrate(float duration) {
    env->CallStaticVoidMethod(datas->cls, datas->vibrate, duration);
}
