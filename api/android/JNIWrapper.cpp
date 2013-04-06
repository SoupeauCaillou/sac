#include "JNIWrapper.h"

jclass JNIHelper::findClass(JNIEnv* env, const std::string& className) {
	jclass c = env->FindClass(className.c_str());
	if (!c) {
		LOGF("Unable to find Java class '" << className << "'");
		return 0;
	} else {
		return (jclass)env->NewGlobalRef(c);
	}
}

jmethodID JNIHelper::findStaticMethod(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
	jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
	if (!mId) {
		LOGF("Unable to find static method '" << name << "' (signature='" << signature << "')");
	}
	return mId;
}

jmethodID JNIHelper::findMethod(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
	jmethodID mId = env->GetMethodID(c, name.c_str(), signature.c_str());
	if (!mId) {
		LOGF("Unable to find method '" << name << "' (signature='" << signature << "')");
	}
	return mId;
}