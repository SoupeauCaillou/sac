#pragma once

#include <stdint.h>
#include <string>

struct FileBuffer {
    FileBuffer() : data(0), size(0) {}
    uint8_t* data;
    int size;
};

class AssetAPI {
    public:
        virtual FileBuffer loadAsset(const std::string& asset) = 0;
};
