#pragma once

#include "../AssetAPI.h"
#include <cstdio>
#include <jni.h>

class AssetAPIAndroidImpl : public AssetAPI {
	public:
		AssetAPIAndroidImpl(JNIEnv *env, jobject assetManager);
		void init();
    	FileBuffer loadAsset(const std::string& asset);
    
    private:
    	JNIEnv *env;
    	jobject assetManager;
    	struct AssetAPIAndroidImplData;
		AssetAPIAndroidImplData* datas;
};
