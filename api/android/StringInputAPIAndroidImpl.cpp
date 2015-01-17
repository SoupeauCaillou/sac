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


#if SAC_ANDROID

#include "StringInputAPIAndroidImpl.h"
#include "base/Log.h"
#include <iostream>

StringInputAPIAndroidImpl::StringInputAPIAndroidImpl() : JNIWrapper<jni_name_api::Enum>("net/damsy/soupeaucaillou/api/StringInputAPI", true) {
        declareMethod(jni_name_api::AskUserInput, "showPlayerKeyboardUI", "()V");
    declareMethod(jni_name_api::Done, "askUserInput", "()Ljava/lang/String;");
    declareMethod(jni_name_api::CancelUserInput, "closePlayerKeyboardUI", "()V");
    declareMethod(jni_name_api::SetNamesList, "setNamesList", "([Ljava/lang/String;)V");
}

void StringInputAPIAndroidImpl::askUserInput(const std::string&, const int) {
    env->CallVoidMethod(instance, methods[jni_name_api::AskUserInput]);
}

bool StringInputAPIAndroidImpl::done(std::string& name) {
    jstring n = (jstring) env->CallObjectMethod(instance, methods[jni_name_api::Done]);
    if (n) {
        const char *mfile = env->GetStringUTFChars(n, 0);
        name = mfile;
        LOGI("Entered string: '" << mfile << "'");
        env->ReleaseStringUTFChars(n, mfile);
        return true;
    }
    return false;
}

void StringInputAPIAndroidImpl::cancelUserInput() {
    env->CallVoidMethod(instance, methods[jni_name_api::CancelUserInput]);
}

void StringInputAPIAndroidImpl::setNamesList(const std::vector<std::string> & names) {
    jclass stringObject = env->FindClass("java/lang/String");
    jobjectArray stringArray = env->NewObjectArray(names.size(), stringObject, NULL);
    for (unsigned i = 0; i < names.size(); ++i) {
        jstring javaname = env->NewStringUTF(names[i].c_str());
        env->SetObjectArrayElement(stringArray, i, javaname);
    }
    env->CallVoidMethod(instance, methods[jni_name_api::SetNamesList], stringArray);
}
#endif
