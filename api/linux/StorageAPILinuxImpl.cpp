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

#include "base/Log.h"

#include <sstream>

#include <sys/stat.h>
#include <sys/types.h>

//for getenv
#include <cstdlib>
//for strcmp
#include <string.h>

 const std::string &  StorageAPILinuxImpl::getDatabasePath() {
    static std::string path;

    if (path.empty()) {
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
                LOGF("Failed to create : '" << ss.str() << "'")
            }
        }

        path = ss.str();
    }
    return path;
 }
