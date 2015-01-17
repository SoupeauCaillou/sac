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