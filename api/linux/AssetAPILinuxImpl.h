#pragma once

#include "../AssetAPI.h"
#include <cstdio>

class AssetAPILinuxImpl : public AssetAPI {
    FileBuffer loadAsset(const std::string& asset);
};
