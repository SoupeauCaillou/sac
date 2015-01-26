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

#include <vector>
#include <string>

#include "util/StorageProxy.h"

class AssetAPI;

class StorageAPI {
    public:
        virtual ~StorageAPI() {}
        virtual void init(AssetAPI * assetAPI, const std::string & databaseName) = 0;

        virtual void setOption(const std::string & name, const std::string & valueIfExisting, const std::string & valueIfNotExisting) = 0;
        virtual std::string getOption(const std::string & name) = 0;
        virtual bool isOption(const std::string & name, const std::string & compareValue) = 0;

        virtual void createTable(IStorageProxy * proxy) = 0;
        virtual void saveEntries(IStorageProxy * proxy) = 0;
        virtual void loadEntries(IStorageProxy * proxy, const std::string & selectArg, const std::string & options = "") = 0;
        virtual void dropAll(IStorageProxy* proxy) = 0;

        virtual int count(IStorageProxy * proxy, const std::string & selectArg, const std::string & options = "") = 0;

        virtual float sum(IStorageProxy * proxy, const std::string & selectArg, const std::string & options = "") = 0;
};
