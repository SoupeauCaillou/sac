#include "SqliteStorageAPIImpl.h"
#include <string>
#include <sstream>

#include <sqlite3.h>

#include "base/Log.h"

#include <sys/stat.h>
#include <sys/types.h>

//for getenv
#include <cstdlib>

//for cerr
#include <iostream>

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

        int i = 0;
        for (; i < argc - 1; i++) {
            (*sav) += argv[i];
            (*sav) += ", ";
        }
        (*sav) += argv[i];

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
        StorageProxy * proxy = static_cast<StorageProxy *> (save);

        proxy->pushAnElement();
        for (int i = 0; i < dataCount; ++i) {
            proxy->setValue(columnName[i], tuple[i]);
        }
        return 0;
    }


///////////////////////////////////////////////////////////////////////////////
///////////////////////End of callbacks////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SqliteStorageAPIImpl::init(AssetAPI * assetAPI, const std::string & databaseName) {
    LOGF_IF(_initialized, "The database has already been initialized!");

    _initialized = true;

    _assetAPI = assetAPI;

    std::stringstream ss;
    ss << _assetAPI->getWritableAppDatasPath() << "/" << databaseName <<".db";
    _dbPath = ss.str();

    //test if we can connect to the db
    bool r = request("", 0, 0);

    if (r) {
        LOGI("initializing database...")

        request("create table info(opt varchar2(10) primary key, value varchar2(10))", 0, 0);

        checkInTable("sound", std::string(), "on");
        checkInTable("gameCount", std::string(), "0");
    }
}

void SqliteStorageAPIImpl::checkInTable(const std::string & option,
const std::string & valueIfExist, const std::string & valueIf404) {
    std::string lookFor = "select value from info where opt like '" + option + "'";
    std::string res;

    request(lookFor, &res, 0);

    //it doesn't exist yet
    if (res.length() == 0 && !valueIf404.empty()) {
        lookFor = "insert into info values('" + option + "', '" + valueIf404 + "')";
        request(lookFor, 0, 0);
    //it exist - need to be updated?
    } else if (res.length() != 0 && !valueIfExist.empty()) {
        lookFor = "update info set value='" + valueIfExist + "' where opt='" + option + "'";
        request(lookFor, 0, 0);
    }
}


bool SqliteStorageAPIImpl::request(const std::string & statement, void* res, int (*completionCallback)(void*,int,char**,char**)) {
    LOGF_IF(!_initialized, "The database hasn't been initialized before first request!");

    sqlite3 *db;
    char *zErrMsg = 0;

    int rc = sqlite3_open(_dbPath.c_str(), &db);
    if( rc ){
        sqlite3_close(db);
        LOGF("Can't open database " << _dbPath << " : " << sqlite3_errmsg(db))
        return false;
    }

    //if we want to use a custom callback
    if (completionCallback && res) {
        rc = sqlite3_exec(db, statement.c_str(), completionCallback, res, &zErrMsg);
    //else use the default one
    } else {
        rc = sqlite3_exec(db, statement.c_str(), callback, res, &zErrMsg);
    }

    if (rc!=SQLITE_OK) {
        LOGE("SQL error: " << zErrMsg << "(asked = " << statement << ')')
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
    return true;
}

void SqliteStorageAPIImpl::setOption(const std::string & name, const std::string & value) {
    checkInTable(name, value, value);
}
std::string SqliteStorageAPIImpl::getOption(const std::string & name) {
    std::string lookFor = "select value from info where opt like '" + name + "'";
    std::string res;

    request(lookFor, &res, 0);
    return res;
}
bool SqliteStorageAPIImpl::isOption(const std::string & name, const std::string & compareValue) {
    return (getOption(name) == compareValue);
}

void SqliteStorageAPIImpl::createTable(StorageProxy * pproxy) {
    std::stringstream ss;
    char separator = ' ';

    ss << "create table " << pproxy->getTableName() << "(";
    for (auto name : pproxy->getColumnsNameAndType()) {
        //hack, used to set the first separator between first and second element (not before the first)
        if (! name.first.empty()) {
            ss << separator << " " << name.first << " " << name.second;
            separator = ',';
        }
    }
    ss << ")";

    request(ss.str(), 0, 0);
}

void SqliteStorageAPIImpl::saveEntries(StorageProxy * pproxy) {
    do {
        std::stringstream ss;
        char separator = ' ';

        ss << "insert into " << pproxy->getTableName() << " values (";
        for (auto name : pproxy->getColumnsNameAndType()) {
            //hack, used to set the first separator between first and second element (not before the first)
            if (! name.first.empty()) {
                if (name.second == "string")
                    ss << separator << " '" << pproxy->getValue(name.first) << "'";
                else
                    ss << separator << " " << pproxy->getValue(name.first);
                separator = ',';
            }
        }
        ss << ")";
        LOGI("Final statement: " << ss.str());
        request(ss.str(), 0, 0);
    } while (pproxy->popAnElement());
}

void SqliteStorageAPIImpl::loadEntries(StorageProxy * pproxy) {
    request("select * from " + pproxy->getTableName(), pproxy, 0);
}
