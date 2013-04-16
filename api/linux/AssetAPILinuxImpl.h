#pragma once

#include "../AssetAPI.h"
#include <cstdio>
#include <list>

class AssetAPILinuxImpl : public AssetAPI {
	void init();
    FileBuffer loadAsset(const std::string& asset);

    std::list<std::string> listContent(const std::string& extension, const std::string& subfolder);

    const std::string & getWritableAppDatasPath();
};
