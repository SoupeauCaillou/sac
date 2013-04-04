#include "AssetAPIAndroidImpl.h"
#include "../../base/Log.h"

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGF("JNI Error : could not find method '" << name << "'/'" << signature << "'")
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
		LOGW("failed to load '" << assetName << "'")
		return 0;
	}
}

FileBuffer AssetAPIAndroidImpl::loadAsset(const std::string& asset) {
    LOGI("loadAssetFromJava: " << asset)
    FileBuffer fb;
    fb.data = loadAssetFromJava(env, assetManager, asset, &fb.size, datas->javaAssetApi, datas->assetToByteArray);
    return fb;
}

std::list<std::string> AssetAPIAndroidImpl::listContent(const std::string&, const std::string&) {
    LOGW("TODO");
    return std::list<std::string>();
}
