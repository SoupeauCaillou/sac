#include "AssetAPIAndroidImpl.h"
#include "../../base/Log.h"

AssetAPIAndroidImpl::AssetAPIAndroidImpl(JNIEnv *pEnv, jobject pAssetManager) : env(pEnv), assetManager(pAssetManager) {
	
}

static uint8_t* loadAssetFromJava(JNIEnv *env, jobject assetManager, const std::string& assetName, int* length) {
	jclass util = env->FindClass("net/damsy/soupeaucaillou/tilematch/TilematchJNILib");
	if (!util) {
		LOGW("ERROR - cannot find class (%p)", env);
		return 0;
	}
	jmethodID mid = env->GetStaticMethodID(util, "assetToByteArray", "(Landroid/content/res/AssetManager;Ljava/lang/String;)[B");
    jstring asset = env->NewStringUTF(assetName.c_str());
    jobject _a = env->CallStaticObjectMethod(util, mid, assetManager, asset);

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
    fb.data = loadAssetFromJava(env, assetManager, asset, &fb.size);
    return fb;
}
