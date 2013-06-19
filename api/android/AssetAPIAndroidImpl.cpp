#include "AssetAPIAndroidImpl.h"
#include "base/Log.h"

AssetAPIAndroidImpl::AssetAPIAndroidImpl() : JNIWrapper<jni_asset_api::Enum>("net/damsy/soupeaucaillou/api/AssetAPI", true) {
    declareMethod(jni_asset_api::LoadAsset, "assetToByteArray", "(Ljava/lang/String;)[B");
    declareMethod(jni_asset_api::GetWritableAppDatasPath, "getWritableAppDatasPath", "()Ljava/lang/String");
}

static uint8_t* loadAssetFromJava(JNIEnv *env, const std::string& assetName, int* length, jobject instance, jmethodID mid) {
    jstring asset = env->NewStringUTF(assetName.c_str());
    jobject _a = env->CallObjectMethod(instance, mid, asset);

	if (_a) {
		jbyteArray a = (jbyteArray)_a;
		*length = env->GetArrayLength(a);
		jbyte* res = new jbyte[*length + 1];
		env->GetByteArrayRegion(a, 0, *length, res);
		res[*length] = '\0';
        LOGV(1, "Loaded asset '" << assetName << "' -> size=" << *length);
		return (uint8_t*)res;
	} else {
		LOGW("failed to load '" << assetName << "'");
		return 0;
	}
}

FileBuffer AssetAPIAndroidImpl::loadAsset(const std::string& asset) {
    LOGI("loadAssetFromJava: " << asset);
    FileBuffer fb;
    fb.data = loadAssetFromJava(env, asset, &fb.size, instance, methods[jni_asset_api::LoadAsset]);
    return fb;
}

std::list<std::string> AssetAPIAndroidImpl::listAssetContent(const std::string&, const std::string&) {
    LOGW("TODO");
    return std::list<std::string>();
}

const std::string &  AssetAPIAndroidImpl::getWritableAppDatasPath() {
    static std::string path;

    jobject result = env->CallObjectMethod(instance, methods[jni_asset_api::GetWritableAppDatasPath]);
    static const char* str = env->GetStringUTFChars((jstring) result, NULL);
    path.assign(str);

    LOGV(1, "path is '" << path << "'.");
    return path;
 }

