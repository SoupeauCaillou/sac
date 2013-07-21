#pragma once

#include "api/WWWAPI.h"

#include "JNIWrapper.h"

#include "api/AssetAPI.h"

namespace jni_www_api {
    enum Enum {
        DownloadFile,
    };
}

class WWWAPIAndroidImpl : public WWWAPI, public JNIWrapper<jni_www_api::Enum> {
    public:
        WWWAPIAndroidImpl();
        FileBuffer downloadFile(const std::string &url);
};
