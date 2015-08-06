#pragma once

class AssetAPI;

#include "util/MurmurHash.h"
#include "util/ResourceHotReload.h"
#include "util/DataFileParser.h"
#include <map>
namespace TuningType
{
	enum Enum {
		Float,
		Int
	};
}

class Tuning : public ResourceHotReload {

public:
	void init(AssetAPI* a) { assetAPI = a; }
	void load(const char* assetName);

	void reload(const char* assetName);

	float f(hash_t h);
	int i(hash_t h);

	void setF(hash_t h, float f);
	void setI(hash_t h, int f);

	const char* asset2FilePrefix() const { return ""; }
    const char* asset2FileSuffix() const { return ""; }

    const std::map<hash_t, TuningType::Enum>&
    	getTypeHints() const { return typeHints; }
private:
	AssetAPI* assetAPI;
	DataFileParser dfp;
	std::map<hash_t, TuningType::Enum> typeHints;
};
