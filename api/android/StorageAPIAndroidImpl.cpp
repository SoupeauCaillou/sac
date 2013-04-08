#include "StorageAPIAndroidImpl.h"
#include "base/Log.h"

StorageAPIAndroidImpl::StorageAPIAndroidImpl()  : JNIWrapper<jni_comm_api::Enum>("net/damsy/soupeaucaillou/api/CommunicationAPI", true) {
    declareMethod(jni_comm_api::GetDatabasePath, "getDatabasePath", "()Ljava/lang/String");
}

void StorageAPIAndroidImpl::init(JNIEnv* pEnv) {
	env = pEnv;
}

const std::string & StorageAPIAndroidImpl::getDatabasePath();
    jobject result = (*env)->CallObjectMethod(instance, methods[jni_comm_api::GetDatabasePath]);
    const char* str = (*env)->GetStringUTFChars(env, (jstring) result, NULL);

    return std::string(str);
}
