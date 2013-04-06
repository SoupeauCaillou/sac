/*
	This file is part of RecursiveRunner.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	RecursiveRunner is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	RecursiveRunner is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "StorageAPILinuxImpl.h"
#include <string>
#include <sstream>
#if ! EMSCRIPTEN
#include <sqlite3.h>
#endif
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


#if ! SAC_EMSCRIPTEN

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
    int callbackColumnNames(void *save, int argc, char **argv, char **){
        std::vector<std::string> *sav = static_cast<std::vector<std::string>*>(save);
        for (int i = 0; i < argc; i++) {
            sav->push_back(argv[i]);
        }
        return 0;
    }
///////////////////////////////////////////////////////////////////////////////
///////////////////////End of callbacks////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool StorageAPILinuxImpl::request(const std::string & statement, void* res, int (*callbackP)(void*,int,char**,char**)) {
    LOGF_IF(!initialized, "The database hasn't been initialized before first request!");

	sqlite3 *db;
	char *zErrMsg = 0;

	int rc = sqlite3_open(_dbPath.c_str(), &db);
	if( rc ){
		LOGE("Can't open database " << _dbPath << " : " << sqlite3_errmsg(db))
		sqlite3_close(db);
		return false;
	}

	//si on veut notre callback personnel(script component)
	if (callbackP && res) {
		rc = sqlite3_exec(db, statement.c_str(), callbackP, res, &zErrMsg);
	//sinon on prend celui par défaut
	} else {
		rc = sqlite3_exec(db, statement.c_str(), callback, res, &zErrMsg);
	}

	if( rc!=SQLITE_OK ){
		LOGE("SQL error: " << zErrMsg << "(asked = " << statement << ')')
		sqlite3_free(zErrMsg);
	}

	sqlite3_close(db);
	return true;
}

void StorageAPILinuxImpl::checkInTable(const std::string & option,
const std::string & valueIfExist, const std::string & valueIf404) {
	std::string lookFor = "select value from info where opt like '" + option + "'";
	std::string res;

	request(lookFor, &res, 0);

	//it doesn't exist yet
	if (res.length() == 0 && valueIf404 != "(null)") {
		lookFor = "insert into info values('" + option + "', '" +
		valueIf404 + "')";
		request(lookFor, 0, 0);

	//it exist - need to be updated?
	} else if (res.length() != 0 && valueIfExist != "(null)") {
		lookFor = "update info set value='" + valueIfExist
		+ "' where opt='" + option + "'";
		request(lookFor, 0, 0);
	}
}
#endif

void StorageAPILinuxImpl::init(const std::string & databaseName) {
    initialized = true;

#if EMSCRIPTEN
    muted = false;
#else
    std::stringstream ss;
    char * pPath = getenv ("XDG_DATA_HOME");
    if (pPath) {
        ss << pPath;
    } else if ((pPath = getenv ("HOME")) != 0) {
        ss << pPath << "/.local/share/";
    } else {
        ss << "/tmp/";
    }
    ss << "sac/";

    // create folder if needed
    struct stat statFolder;
    int st = stat(ss.str().c_str(), &statFolder);
    if (st || (statFolder.st_mode & S_IFMT) != S_IFDIR) {
        if (mkdir(ss.str().c_str(), S_IRWXU | S_IWGRP | S_IROTH)) {
            LOGE("Failed to create : '" << ss.str() << "'")
            return;
        }
    }

    ss << databaseName <<".db";
    _dbPath = ss.str();

    //test if we can connect to the db
    bool r = request("", 0, 0);

    if (r) {
        LOGI("initializing database...")

        request("create table info(opt varchar2(8), value varchar2(11), constraint f1 primary key(opt,value))", 0, 0);

        checkInTable("sound", "(null)", "on");
        checkInTable("gameCount", "(null)", "0");
    }
#endif
}

bool StorageAPILinuxImpl::isFirstGame() {
#if SAC_EMSCRIPTEN
    return false;
#else
    std::string s;

    request("select value from info where opt like 'gameCount'", &s, 0);

    return (s == "1");
#endif
}

bool StorageAPILinuxImpl::isMuted() {
#if SAC_EMSCRIPTEN
    return true;
#else
    std::string s;
    request("select value from info where opt like 'sound'", &s, 0);
    return (s == "off");
#endif
}

void StorageAPILinuxImpl::setMuted(bool b) {
#if SAC_EMSCRIPTEN
    muted = b;
#else
    std::stringstream req;
    req << "UPDATE info SET value='" << (b ? "off" : "on") << "' where opt='sound'";
    request(req.str(),0, 0);
#endif
}

void StorageAPILinuxImpl::incrementGameCount() {
#if ! SAC_EMSCRIPTEN
	std::string gameCount;
	request("select value from info where opt='gameCount'", &gameCount, 0);
	LOGV(1, "Game count: " << gameCount)
	std::stringstream ss;
	ss << atoi(gameCount.c_str()) + 1;

	std::cout << gameCount.c_str() << "->" << ss.str().c_str() << std::endl;
	request("update info set value='" + ss.str() + "' where opt='gameCount'", 0, 0);
#endif
}
