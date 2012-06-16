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
#include "NameInputAPIAndroidImpl.h"
#include "base/Log.h"
#include <iostream>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}


struct NameInputAPIAndroidImpl::NameInputAPIAndroidImplDatas {
    jclass javaNameApi;
    jmethodID show;
    jmethodID hide;
    jmethodID done;
    bool initialized;
};

NameInputAPIAndroidImpl::NameInputAPIAndroidImpl() {
	datas = new NameInputAPIAndroidImplDatas();
	datas->initialized = false;
}

NameInputAPIAndroidImpl::~NameInputAPIAndroidImpl() {
    uninit();
    delete datas;
}

void NameInputAPIAndroidImpl::uninit() {
	if (datas->initialized) {
		env->DeleteGlobalRef(datas->javaNameApi);
		datas->initialized = false;
	}
}

void NameInputAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;
    
    datas->javaNameApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/heriswap/HeriswapJNILib"));
    datas->show = jniMethodLookup(env, datas->javaNameApi, "showPlayerNameUi", "()V");
    datas->done = jniMethodLookup(env, datas->javaNameApi, "queryPlayerName", "()Ljava/lang/String;");
    // datas->hide = jniMethodLookup(env, datas->javaNameApi, "", "");
    datas->initialized = true;
}

void NameInputAPIAndroidImpl::show() {
    env->CallStaticVoidMethod(datas->javaNameApi, datas->show);
}

bool NameInputAPIAndroidImpl::done(std::string& name) {
    jstring n = (jstring) env->CallStaticObjectMethod(datas->javaNameApi, datas->done);
    if (n) {
        const char *mfile = env->GetStringUTFChars(n, 0);
        LOGW("name choosen: %s", mfile);
        name = mfile;
        env->ReleaseStringUTFChars(n, mfile);
        return true;
    }
    return false;
}

void NameInputAPIAndroidImpl::hide() {

}
