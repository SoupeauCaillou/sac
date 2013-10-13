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

#include "InAppPurchaseAPIAndroidImpl.h"

InAppPurchaseAPIAndroidImpl::InAppPurchaseAPIAndroidImpl() : JNIWrapper<jni_inapp_api::Enum>(
    "net/damsy/soupeaucaillou/api/InAppPurchaseAPI", true) {
   declareMethod(jni_inapp_api::Purchase, "Purchase", "(Ljava/lang/String;)V");
}



void InAppPurchaseAPIAndroidImpl::purchase(const std::string & name) {
    LOGI(name << " and " << env);
    jstring jname = env->NewStringUTF(name.c_str());
    LOGI(jname);
    return env->CallVoidMethod(instance, methods[jni_inapp_api::Purchase], jname);
}
