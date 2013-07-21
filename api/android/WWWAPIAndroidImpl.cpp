#include "WWWAPIAndroidImpl.h"
#include "base/Log.h"

WWWAPIAndroidImpl::WWWAPIAndroidImpl() : JNIWrapper<jni_www_api::Enum>("net/damsy/soupeaucaillou/api/WWWAPI", true) {
    declareMethod(jni_www_api::DownloadFile,
        "fileToByteArray", "(Ljava/lang/String;)[B");
}

static uint8_t* loadFileFromJava(JNIEnv *env, const std::string& url, int* length, jobject instance, jmethodID mid) {
    jstring asset = env->NewStringUTF(url.c_str());
    jobject _a = env->CallObjectMethod(instance, mid, asset);

	if (_a) {
		jbyteArray a = (jbyteArray)_a;
		*length = env->GetArrayLength(a);
		jbyte* res = new jbyte[*length + 1];
		env->GetByteArrayRegion(a, 0, *length, res);
		res[*length] = '\0';
        LOGV(1, "Loaded url '" << url << "' -> size=" << *length);
		return (uint8_t*)res;
	} else {
		LOGW("failed to load '" << url << "'");
		return 0;
	}
}

FileBuffer WWWAPIAndroidImpl::downloadFile(const std::string& url) {
    LOGI("downloadFile: " << url);
    FileBuffer fb;
    fb.data = loadFileFromJava(env, url, &fb.size, instance, methods[jni_www_api::DownloadFile]);
    return fb;
}
