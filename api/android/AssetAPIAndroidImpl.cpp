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



#include "AssetAPIAndroidImpl.h"
#include "base/Log.h"

AssetAPIAndroidImpl::AssetAPIAndroidImpl() : JNIWrapper<jni_asset_api::Enum>("net/damsy/soupeaucaillou/api/AssetAPI", true) {
    declareMethod(jni_asset_api::LoadAsset,
        "assetToByteArray", "(Ljava/lang/String;)[B");
    declareMethod(jni_asset_api::GetWritableAppDatasPath,
        "getWritableAppDatasPath", "()Ljava/lang/String;");
    declareMethod(jni_asset_api::ListAssetContent,
        "listAssetContent", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;");
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

std::list<std::string> AssetAPIAndroidImpl::listAssetContent(const std::string& extension, const std::string& path) {
    // Build 2 java strings from args
    jstring jExt = env->NewStringUTF(extension.c_str());
    jstring jPath = env->NewStringUTF(path.c_str());

    // Call Java method
    jobjectArray result = (jobjectArray)env->CallObjectMethod(instance, methods[jni_asset_api::ListAssetContent], jExt, jPath);

    std::list<std::string> r;
    // Parse result
    if (!result) {
        LOGW("ListAssetContent returned null array");
        return r;
    } else {
        int stringCount = env->GetArrayLength(result);

        for (int i=0; i<stringCount; i++) {
            jstring string = (jstring) env->GetObjectArrayElement(result, i);
            const char *rawString = env->GetStringUTFChars(string, 0);
            r.push_back(std::string(rawString));
            env->ReleaseStringUTFChars(string, rawString);
        }
    }
    return r;
}

const std::string &  AssetAPIAndroidImpl::getWritableAppDatasPath() {
    static std::string path;

    jobject result = env->CallObjectMethod(instance, methods[jni_asset_api::GetWritableAppDatasPath]);
    static const char* str = env->GetStringUTFChars((jstring) result, NULL);
    path.assign(str);

    LOGV(1, "path is '" << path << "'.");
    return path;
 }

FileBuffer AssetAPIAndroidImpl::loadFile(const std::string&) {
    LOGT("Extract Linux impl as static method and use it here");
    return FileBuffer();
}

std::list<std::string> AssetAPIAndroidImpl::listContent(const std::string&, const std::string&, const std::string&) {
    LOGT("Extract Linux impl as static method and use it here");
    return std::list<std::string>();
}
