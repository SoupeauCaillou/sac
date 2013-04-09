#include "LocalizeAPIAndroidImpl.h"
#include "base/Log.h"
#include <map>

LocalizeAPIAndroidImpl::LocalizeAPIAndroidImpl() : JNIWrapper<jni_loc_api::Enum>("net/damsy/soupeaucaillou/api/LocalizeAPI", true) {
	declareMethod(jni_loc_api::Text, "localize", "(Ljava/lang/String;)Ljava/lang/String;");
}

std::string LocalizeAPIAndroidImpl::text(const std::string& s) {
	std::map<std::string, std::string>::iterator it = cache.find(s);
	if (it != cache.end()) {
		return it->second;
	}

	jstring name = env->NewStringUTF(s.c_str());
	jstring result = (jstring)env->CallObjectMethod(instance, methods[jni_loc_api::Text], name);

	const char *loc = env->GetStringUTFChars(result, 0);
	std::string b(loc);
	env->ReleaseStringUTFChars(result, loc);

	cache.insert(std::make_pair(s, b));
	return b;
}
