#pragma once

#include "../AssetAPI.h"
#include "JNIWrapper.h"


namespace jni_asset_api {
    enum Enum {
        LoadAsset,
        ListContent,
    };
}

class AssetAPIAndroidImpl : public AssetAPI, public JNIWrapper<jni_asset_api::Enum> {
	public:
		AssetAPIAndroidImpl();

        std::list<std::string > listContent(const std::string&, const std::string&);

    	FileBuffer loadAsset(const std::string& asset);
};
