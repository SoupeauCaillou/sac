#include "StorageAPIAndroidImpl.h"
#include "base/Log.h"

StorageAPIAndroidImpl::StorageAPIAndroidImpl()  : JNIWrapper<jni_stor_api::Enum>("net/damsy/soupeaucaillou/api/StorageAPI", true) {
    declareMethod(jni_stor_api::GetDatabasePath, "getDatabasePath", "()Ljava/lang/String");
}

void StorageAPIAndroidImpl::init(JNIEnv* pEnv) {
	env = pEnv;
}

const std::string & StorageAPIAndroidImpl::getDatabasePath() {
    static std::string path;

    jobject result = env->CallObjectMethod(instance, methods[jni_stor_api::GetDatabasePath]);
    static const char* str = env->GetStringUTFChars((jstring) result, NULL);
    path.assign(str);

    LOGV(1, "path is '" << path << "'.");
    return path;
}
