#pragma once

#include <vector>
#include <string>

#include "api/StorageAPI.h"
#include "api/AssetAPI.h"

#include "util/StorageProxy.h"

class SqliteStorageAPIImpl : public StorageAPI {
	public:
        SqliteStorageAPIImpl() : _initialized(false) {}

        void init(AssetAPI * assetAPI, const std::string & databaseName);

        void setOption(const std::string & name, const std::string & value);
        std::string getOption(const std::string & name);
        bool isOption(const std::string & name, const std::string & compareValue);

        void createTable(StorageProxy * proxy);
        void saveEntries(StorageProxy * proxy);
        void loadEntries(StorageProxy * proxy);

    private:
        bool request(const std::string & statement, void* res, int (*completionCallback)(void*,int,char**,char**));
        void checkInTable(const std::string & option, const std::string & valueIfExist, const std::string & valueIf404);
        std::string _dbPath;

        bool _initialized;
        AssetAPI* _assetAPI;
};
