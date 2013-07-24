#pragma once

#include "../AssetAPI.h"
#include "JNIWrapper.h"

#include "base/Log.h"

namespace jni_asset_api {
    enum Enum {
        LoadAsset,
        ListAssetContent,
        GetWritableAppDatasPath,
    };
}

class AssetAPIAndroidImpl : public AssetAPI, public JNIWrapper<jni_asset_api::Enum> {
	public:
		AssetAPIAndroidImpl();

        std::list<std::string> listAssetContent(const std::string& extension, const std::string& subfolder);

    	FileBuffer loadAsset(const std::string& asset);

        const std::string & getWritableAppDatasPath();

        FileBuffer loadFile(const std::string&);

        std::list<std::string> listContent(const std::string&, const std::string&,
            const std::string&);

        void createDirectory(const std::string& ) { LOGT(""); }
        bool doesExistFileOrDirectory(const std::string& ) { LOGT(""); return false; }
        void removeFileOrDirectory(const std::string& ) { LOGT(""); }


        void synchronize() {}
};
