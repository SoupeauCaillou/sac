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
