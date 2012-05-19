#pragma once

#include "../AssetAPI.h"
#include <cstdio>

class AssetAPILinuxImpl : public AssetAPI {
	void init();
    FileBuffer loadAsset(const std::string& asset);
};
