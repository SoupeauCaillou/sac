#pragma once

#include <vector>
#include <string>

#include "util/StorageProxy.h"

class AssetAPI;

class StorageAPI {
	public:
        virtual void init(AssetAPI * assetAPI, const std::string & databaseName) = 0;

        virtual void setOption(const std::string & name, const std::string & value) = 0;
        virtual std::string getOption(const std::string & name) = 0;
        virtual bool isOption(const std::string & name, const std::string & compareValue) = 0;

        virtual void createTable(StorageProxy * proxy) = 0;
        virtual void saveEntries(StorageProxy * proxy) = 0;
        virtual void loadEntries(StorageProxy * proxy) = 0;
};
