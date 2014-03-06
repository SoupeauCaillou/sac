/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#include <string>

class AssetAPI;

#include "api/StorageAPI.h"

#include "util/StorageProxy.h"

class SqliteStorageAPIImpl : public StorageAPI {
	public:
        SqliteStorageAPIImpl() : initialized(false) {}

        void init(AssetAPI * assetAPI, const std::string & databaseName);

        void setOption(const std::string & name, const std::string & valueIfExisting, const std::string & valueIfNotExisting);
        std::string getOption(const std::string & name);
        bool isOption(const std::string & name, const std::string & compareValue);

        void createTable(IStorageProxy * proxy);
        void saveEntries(IStorageProxy * proxy);
        void loadEntries(IStorageProxy * proxy, const std::string & selectArg = "*", const std::string & options = "");

        int count(IStorageProxy * proxy, const std::string & selectArg, const std::string & options = "");

        float sum(IStorageProxy * proxy, const std::string & selectArg, const std::string & options = "");

    private:
        bool request(const std::string & statement, void* res, int (*completionCallback)(void*,int,char**,char**));
        void checkInTable(const std::string & option, const std::string & valueIfExist, const std::string & valueIf404);
        void createTable(const std::string & tableName, const std::string & statement);
        
        std::string databasePath;
        bool initialized;

        /*Needed to get the datapath on filesystem which is platform dependent*/
        AssetAPI* assetAPI;
};
