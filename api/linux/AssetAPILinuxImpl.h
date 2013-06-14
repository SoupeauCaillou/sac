#pragma once

#include "../AssetAPI.h"
#include <cstdio>
#include <list>

class AssetAPILinuxImpl : public AssetAPI {
	void init();

    FileBuffer loadFile(const std::string& full);
    FileBuffer loadAsset(const std::string& asset);

    std::list<std::string> listContent(const std::string& directory, const std::string& extension, const std::string& subfolder);
    std::list<std::string> listAssetContent(const std::string& extension, const std::string& subfolder);

    const std::string & getWritableAppDatasPath();

    void synchronize();
};
