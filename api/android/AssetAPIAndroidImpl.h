#pragma once

#include "../AssetAPI.h"
#include <cstdio>
#include <jni.h>

class AssetAPIAndroidImpl : public AssetAPI {
	public:
		AssetAPIAndroidImpl();
        ~AssetAPIAndroidImpl();
		void init(JNIEnv *env, jobject assetManager);
		void uninit();

    	FileBuffer loadAsset(const std::string& asset);

    private:
    	JNIEnv *env;
    	jobject assetManager;
    	struct AssetAPIAndroidImplData;
		AssetAPIAndroidImplData* datas;
};
