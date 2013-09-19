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
