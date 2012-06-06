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
};

AssetAPIAndroidImpl::AssetAPIAndroidImpl(JNIEnv *pEnv, jobject pAssetManager) : env(pEnv), assetManager(pAssetManager) {
	
}

AssetAPIAndroidImpl::~AssetAPIAndroidImpl() {
    env->DeleteGlobalRef(datas->javaAssetApi);
    delete datas;
}

void AssetAPIAndroidImpl::init() {
	datas = new AssetAPIAndroidImplData();

	datas->javaAssetApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/heriswap/HeriswapJNILib"));
	datas->assetToByteArray = jniMethodLookup(env, datas->javaAssetApi, "assetToByteArray", "(Landroid/content/res/AssetManager;Ljava/lang/String;)[B");
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
