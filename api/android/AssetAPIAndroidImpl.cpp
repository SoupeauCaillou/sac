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
#include "AssetAPIAndroidImpl.h"
#include "../../base/Log.h"

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}

struct AssetAPIAndroidImpl::AssetAPIAndroidImplData {	
	jclass javaAssetApi;
	jmethodID assetToByteArray;
	bool initialized;
};

AssetAPIAndroidImpl::AssetAPIAndroidImpl() {
	datas = new AssetAPIAndroidImplData();
	datas->initialized = false;
}

AssetAPIAndroidImpl::~AssetAPIAndroidImpl() {
    uninit();
    delete datas;
}

void AssetAPIAndroidImpl::init(JNIEnv *pEnv, jobject pAssetManager) {
	assetManager = pAssetManager;
	env = pEnv;

	datas->javaAssetApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/AssetAPI"));
	datas->assetToByteArray = jniMethodLookup(env, datas->javaAssetApi, "assetToByteArray", "(Landroid/content/res/AssetManager;Ljava/lang/String;)[B");
	datas->initialized = true;
}

void AssetAPIAndroidImpl::uninit() {
	if (datas->initialized) {
		env->DeleteGlobalRef(datas->javaAssetApi);
		datas->initialized = false;
	}
}

static uint8_t* loadAssetFromJava(JNIEnv *env, jobject assetManager, const std::string& assetName, int* length, jclass cls, jmethodID mid) {
    jstring asset = env->NewStringUTF(assetName.c_str());
    jobject _a = env->CallStaticObjectMethod(cls, mid, assetManager, asset);

	if (_a) {
		jbyteArray a = (jbyteArray)_a;
		*length = env->GetArrayLength(a);
		jbyte* res = new jbyte[*length + 1];
		env->GetByteArrayRegion(a, 0, *length, res);
		res[*length] = '\0';
		return (uint8_t*)res;
	} else {
		LOGW("%s failed to load '%s'\n", __FUNCTION__, assetName.c_str());
		return 0;
	}
}

FileBuffer AssetAPIAndroidImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
    fb.data = loadAssetFromJava(env, assetManager, asset, &fb.size, datas->javaAssetApi, datas->assetToByteArray);
    return fb;
}
