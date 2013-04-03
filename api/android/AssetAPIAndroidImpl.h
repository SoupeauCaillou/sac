#pragma once

#include "../AssetAPI.h"
#include <cstdio>
#include <jni.h>
#include <string>

class AssetAPIAndroidImpl : public AssetAPI {
	public:
		AssetAPIAndroidImpl();
        ~AssetAPIAndroidImpl();
		void init(JNIEnv *env, jobject assetManager);
		void uninit();
        std::list<std::string > listContent(const std::string&, const std::string&);

    	FileBuffer loadAsset(const std::string& asset);

    private:
    	JNIEnv *env;
    	jobject assetManager;
    	struct AssetAPIAndroidImplData;
		AssetAPIAndroidImplData* datas;
};
