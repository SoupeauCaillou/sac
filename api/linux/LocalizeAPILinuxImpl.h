#pragma once

#include "../LocalizeAPI.h"

#include <map>

class AssetAPI;

class LocalizeAPILinuxImpl : public LocalizeAPI {
	private:
		std::map<std::string, std::string> _idToMessage;
	public:
		int init(AssetAPI* assetAPI, const std::string & lang);
		std::string text(const std::string& s);
};
