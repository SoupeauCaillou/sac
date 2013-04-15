#pragma once

#include <vector>
#include <string>

#include "util/StorageProxy.h"

class StorageAPI {
	public:
		virtual const std::string & getDatabasePath() = 0;

        void setOption(const std::string & name, const std::string & value);

        std::string getOption(const std::string & name);

        bool isOption(const std::string & name, const std::string & compareValue);

        void createTable(StorageProxy * proxy);

        void getEntries(StorageProxy * proxy);

    private:
        std::string _dbPath;
};
