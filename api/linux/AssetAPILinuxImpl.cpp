#include "AssetAPILinuxImpl.h"

FileBuffer AssetAPILinuxImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
    std::string full = "assets/" + asset;
    FILE* file = fopen(full.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    fb.size = ftell(file);
    rewind(file);
    fb.data = new uint8_t[fb.size];
    fread(fb.data, fb.size, 1, file);
    fclose(file);
    return fb;
}
