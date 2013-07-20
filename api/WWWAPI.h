#pragma once

#include <cstdio>
#include <string>

class WWWAPI {
    public:
        //return 0 if any error, else return the downloaded file
        //if filename is null, we'll create a temporary file. You must close it!
        virtual FILE* downloadFile(const std::string & url, FILE* filename) = 0;
};
