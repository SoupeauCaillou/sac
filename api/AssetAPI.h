#pragma once

#include <stdint.h>
#include <string>

struct FileBuffer {
    uint8_t* data;
    int size;
};

class AssetAPI {
    public:
    	virtual void init() = 0;
        virtual FileBuffer loadAsset(const std::string& asset) = 0;
};
