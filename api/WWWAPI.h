#pragma once

#include <cstdio>
#include <string>

struct FileBuffer;

class WWWAPI {
    public:
        //return 0 if any error, else return the downloaded file
        //if filename is null, we'll create a temporary file. You must close it!
        virtual FileBuffer downloadFile(const std::string &url) = 0;
};
