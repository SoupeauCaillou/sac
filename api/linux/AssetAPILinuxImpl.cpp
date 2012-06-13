#include "AssetAPILinuxImpl.h"

void AssetAPILinuxImpl::init() {
	
}
#include <iostream>
FileBuffer AssetAPILinuxImpl::loadAsset(const std::string& asset) {
    FileBuffer fb;
    fb.data = 0;
    std::string full = "assets/" + asset;
    FILE* file = fopen(full.c_str(), "rb");
    if (!file)
        return fb;
    fseek(file, 0, SEEK_END);
    fb.size = ftell(file);
    rewind(file);
    fb.data = new uint8_t[fb.size + 1];
    int count = 0;
do {
count += fread(&fb.data[count], 1, fb.size - count, file);
} while (count < fb.size);

    fclose(file);
    fb.data[fb.size] = 0;
    std::cout << asset << ":" << fb.size  << "/" << count << std::endl;
    return fb;
}
