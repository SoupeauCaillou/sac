#pragma once

#include "api/WWWAPI.h"
class WWWAPIcURLImpl : public WWWAPI {
    public:
        FileBuffer* downloadFile(const std::string &url, FileBuffer *fb);
};
