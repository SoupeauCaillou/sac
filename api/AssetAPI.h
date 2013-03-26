#pragma once

#include <stdint.h>
#include <string>
#include <list>

struct FileBuffer {
    FileBuffer() : data(0), size(0) {}
    uint8_t* data;
    int size;
};

class AssetAPI {
    public:
        virtual FileBuffer loadAsset(const std::string& asset) = 0;
        virtual std::list<std::string> listContent(const std::string& extension, const std::string& subfolder) = 0;
};
