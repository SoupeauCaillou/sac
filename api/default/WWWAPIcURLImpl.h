#pragma once

#include "api/WWWAPI.h"
class WWWAPIcURLImpl : public WWWAPI {
    public:
        FILE* downloadFile(const std::string & url, FILE* filename);
};
