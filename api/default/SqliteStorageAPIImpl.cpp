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



#include "SqliteStorageAPIImpl.h"

#include "base/Log.h"
#if !SAC_EMSCRIPTEN
#include <sqlite3.h>

#include <sys/stat.h>
#include <sys/types.h>

//for getenv
#include <cstdlib>

//for cerr
#include <iostream>
#include <string>
#include <sstream>

//for strcmp
#include <string.h>
//for sscanf
#include <stdio.h>
#include <vector>


///////////////////////////////////////////////////////////////////////////////
///////////////////////Callbacks for sqlite datas treatment////////////////////
///////////////////////////////////////////////////////////////////////////////
    //convert tuple into string with format "res1, res2, res3, ..."
    int callback(void *save, int argc, char **argv, char **){
        std::string *sav = static_cast<std::string*>(save);
        sav->clear();

        int i = 0;
        for (; i < argc - 1; i++) {
            sav->append(argv[i]);
            sav->append(", ");
        }
        sav->append(argv[i]);
#if SAC_ENABLE_LOG
        LOGI("query string result: " << *sav);
#endif

        return 0;
    }

    //return columns' names of the table
    int callbackColumnNames(void *save, int dataCount, char **tuple, char **) {
        std::vector<std::string> *sav = static_cast<std::vector<std::string>*>(save);
        for (int i = 0; i < dataCount; ++i) {
            sav->push_back(tuple[i]);
        }
        return 0;
    }

    //convert a tuple to proxy
    int callbackProxyConversion(void *save, int dataCount, char **tuple, char ** columnName) {
        IStorageProxy * proxy = static_cast<IStorageProxy *> (save);

        proxy->pushAnElement();
        for (int i = 0; i < dataCount; ++i) {
            proxy->setValue(columnName[i], tuple[i], false);
        }
        return 0;
    }
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////End of callbacks////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SqliteStorageAPIImpl::init(AssetAPI * assetAPI, const std::string & databaseName) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    LOGF_IF(_initialized, "The database has already been initialized!");

    _initialized = true;

    _assetAPI = assetAPI;

    std::stringstream ss;
    ss << _assetAPI->getWritableAppDatasPath() << "/" << databaseName <<".db";
    _dbPath = ss.str();

    //test if we can connect to the db
    bool r = request("", 0, 0);

    if (r) {
        LOGI("initializing database...");

        createTable("info", "opt varchar2(10) primary key, value varchar2(10)");
    }
    #endif
}

void SqliteStorageAPIImpl::createTable(const std::string & tableName, const std::string & statement) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    std::string res;
    request("SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "'", &res, 0);
    if (res.empty()) {
        request("create table " + tableName + "(" + statement + ")", 0, 0);
    }
    #endif
}

bool SqliteStorageAPIImpl::request(const std::string & statement, void* res, int (*completionCallback)(void*,int,char**,char**)) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    LOGF_IF(!_initialized, "The database hasn't been initialized before first request!");

#ifdef SAC_ENABLE_LOG
    LOGI("sqlite request: " << statement);
#endif

    sqlite3 *db;
    char *zErrMsg = 0;

    int rc = sqlite3_open(_dbPath.c_str(), &db);
    if( rc ){
        sqlite3_close(db);
        LOGF("Can't open database " << _dbPath << " : " << sqlite3_errmsg(db));
        return false;
    }

    //if we want to use a custom callback
    if (completionCallback && res) {
        rc = sqlite3_exec(db, statement.c_str(), completionCallback, res, &zErrMsg);
    //else use the default one
    } else {
        rc = sqlite3_exec(db, statement.c_str(), callback, res, &zErrMsg);
    }

    if (rc != SQLITE_OK) {
        LOGE("SQL error: " << zErrMsg << "(asked = " << statement << ')');
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    #endif
    return true;
}

void SqliteStorageAPIImpl::setOption(const std::string & name, const std::string & valueIfExisting, const std::string & valueIfNotExisting) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    std::string lookFor = "select opt from info where opt like '" + name + "'";
    std::string res;
    request(lookFor, &res, 0);

    //it doesn't exist yet
    if (res.length() == 0 && !valueIfNotExisting.empty()) {
        lookFor = "insert into info values('" + name + "', '" + valueIfNotExisting + "')";
        request(lookFor, 0, 0);
    //it exist - need to be updated?
    } else if (res.length() != 0 && !valueIfExisting.empty()) {
        lookFor = "update info set value='" + valueIfExisting + "' where opt='" + name + "'";
        request(lookFor, 0, 0);
    }
    #endif
}

std::string SqliteStorageAPIImpl::getOption(const std::string & name) {
    std::string res;
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    std::string lookFor = "select value from info where opt like '" + name + "'";
    request(lookFor, &res, 0);
    #endif
    return res;
}
bool SqliteStorageAPIImpl::isOption(const std::string & name, const std::string & compareValue) {
    return (getOption(name) == compareValue);
}

void SqliteStorageAPIImpl::createTable(IStorageProxy * pproxy) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    std::stringstream ss;
    char separator = ' ';

    for (auto name : pproxy->getColumnsNameAndType()) {
        //hack, used to set the first separator between first and second element (not before the first)
        if (! name.first.empty()) {
            ss << separator << " " << name.first << " " << name.second;
            separator = ',';
        }
    }
    createTable(pproxy->getTableName(), ss.str());
    #endif
}

void SqliteStorageAPIImpl::saveEntries(IStorageProxy * pproxy) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    if (pproxy->isEmpty())
        return;

    do {
        std::stringstream ss;
        char separator = ' ';

        ss << "insert into " << pproxy->getTableName() << "(";
        std::stringstream ssNames;
        std::stringstream ssValues;

        for (auto name : pproxy->getColumnsNameAndType()) {
            //hack, used to set the first separator between first and second element (not before the first)
            if (! name.first.empty()) {
                ssNames << separator << " '" << name.first << "'";

                if (name.second == "string")
                    ssValues << separator << " '" << pproxy->getValue(name.first) << "'";
                else
                    ssValues << separator << " " << pproxy->getValue(name.first);
                separator = ',';
            }
        }
        ss << ssNames.str() << ") values (" << ssValues.str() << ")";
        LOGI("Final statement: " << ss.str());
        request(ss.str(), 0, 0);

        pproxy->popAnElement();
    } while (!pproxy->isEmpty());
    #endif
}

void SqliteStorageAPIImpl::loadEntries(IStorageProxy * pproxy, const std::string & selectArg, const std::string & options) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    #else
    request("select " +  selectArg + " from " + pproxy->getTableName() + " " + options, pproxy, callbackProxyConversion);
    #endif
}


int SqliteStorageAPIImpl::count(IStorageProxy * pproxy, const std::string & selectArg, const std::string & options) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    return 0;
    #else
    //because callbacks are C still, we must create a temp string variable because we cannot pass iss.str() to the callback
    std::string res;

    //we do select count(*) from (select ...) because for some specific requests it's needed
    //for example, this cannot be done in a single time: select count(*) from (select distinct difficulty, mode from Score);
    request("select count(*) from (select  " +  selectArg + " from " + pproxy->getTableName() + " " + options + ")", &res, 0);
    std::istringstream iss;
    iss.str(res);
    int finalRes;

    iss >> finalRes;

    return finalRes;
    #endif
}

float SqliteStorageAPIImpl::sum(IStorageProxy * pproxy, const std::string & selectArg, const std::string & options) {
    #if SAC_EMSCRIPTEN
    LOGT("sqlite3 support");
    return 0;
    #else
    //because callbacks are C still, we must create a temp string variable because we cannot pass iss.str() to the callback
    std::string res;
    //'ifnull' is needed if there is no score, it will return '0' instead of a null string
    request("select ifnull(sum(" +  selectArg + "), 0) from " + pproxy->getTableName() + " " + options, &res, 0);
    std::istringstream iss;
    iss.str(res);
    float finalRes;

    iss >> finalRes;

    return finalRes;
    #endif
}

