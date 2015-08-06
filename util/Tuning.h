#pragma once

class AssetAPI;

#include "util/MurmurHash.h"
#include "util/ResourceHotReload.h"
#include "util/DataFileParser.h"

class Tuning : public ResourceHotReload {

public:
	void init(AssetAPI* a) { assetAPI = a; }
	void load(const char* assetName);

	void reload(const char* assetName);

	float f(hash_t h);
	int i(hash_t h);

	const char* asset2FilePrefix() const { return ""; }
    const char* asset2FileSuffix() const { return ""; }
private:
	AssetAPI* assetAPI;
	DataFileParser dfp;
};
